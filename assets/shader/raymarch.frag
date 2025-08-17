#version 450
#extension GL_ARB_gpu_shader_int64 : enable

#define BRICK_SIZE 8
#define PI 3.1415926535897932384626433832795

#define MAX_SHADOW_STEPS 100
#define MAX_LIGHT_DIST 1000.0
#define SHADOW_STEP 0.5

layout(location = 0) out vec4 outColor;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform vec2 resolution;

uniform uint brickSize;
uniform ivec3 gridSize;

uniform float time;
uniform float voxelScale;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;

vec4 background = vec4(0.1, 0.1, 0.8, 1.0);

struct Brick {
    uint64_t bitmask[(BRICK_SIZE * BRICK_SIZE * BRICK_SIZE) / 64];
    uint materialOffset;
};

struct MaterialInfo {
    vec4 albedo;
    float metallic;
    float roughness;
};

layout(std430, binding = 0) buffer BrickMapBuffer {
    uint brickMap[];
};

layout(std430, binding = 1) buffer BrickBuffer {
    Brick bricks[];
};

layout(std430, binding = 2) buffer MaterialBuffer {
    uint materials[];
};

layout(std430, binding = 3) buffer MaterialInfosBuffer {
    MaterialInfo materialInfos[];
};

const int MAX_STEPS = 200;
const float MAX_DIST = 1000.0;

struct HitInfo {
    bool hit;
    vec3 position;
    ivec3 voxelPos;
    vec3 normal; // surface normal at hit
};

bool intersectAABB(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax, out float tEnter, out float tExit) {
    const float INF = 1e30;
    const float EPS = 1e-12;

    vec3 t1, t2;

    // X axis
    if (abs(rd.x) > EPS) {
        float inv = 1.0 / rd.x;
        t1.x = (boxMin.x - ro.x) * inv;
        t2.x = (boxMax.x - ro.x) * inv;
    } else {
        if (ro.x < boxMin.x || ro.x > boxMax.x) return false; // Parallel and outside slab
        t1.x = -INF; t2.x = INF; // Parallel and inside slab
    }

    // Y axis
    if (abs(rd.y) > EPS) {
        float inv = 1.0 / rd.y;
        t1.y = (boxMin.y - ro.y) * inv;
        t2.y = (boxMax.y - ro.y) * inv;
    } else {
        if (ro.y < boxMin.y || ro.y > boxMax.y) return false;
        t1.y = -INF; t2.y = INF;
    }

    // Z axis
    if (abs(rd.z) > EPS) {
        float inv = 1.0 / rd.z;
        t1.z = (boxMin.z - ro.z) * inv;
        t2.z = (boxMax.z - ro.z) * inv;
    } else {
        if (ro.z < boxMin.z || ro.z > boxMax.z) return false;
        t1.z = -INF; t2.z = INF;
    }

    vec3 tMin = min(t1, t2);
    vec3 tMax = max(t1, t2);

    tEnter = max(max(tMin.x, tMin.y), tMin.z);
    tExit  = min(min(tMax.x, tMax.y), tMax.z);

    return tExit >= max(tEnter, 0.0);
}

uint getVoxelIndex(ivec3 localPos) {
    return localPos.x + localPos.y * BRICK_SIZE + localPos.z * BRICK_SIZE * BRICK_SIZE;
}

bool isVoxelSolid(uint brickIndex, uint voxelIndex) {
    Brick brick = bricks[brickIndex];
    uint64_t word = brick.bitmask[voxelIndex / 64];
    return ((word >> (voxelIndex % 64)) & 1UL) != 0UL;
}

uint getBrickIndex(ivec3 brickPos) {
    if (any(lessThan(brickPos, ivec3(0))) || any(greaterThanEqual(brickPos, gridSize)))
        return 0xFFFFFFFFu;
    return brickMap[brickPos.x + brickPos.y * gridSize.x + brickPos.z * gridSize.x * gridSize.y];
}

bool isVoxelSolidGlobal(ivec3 globalVoxelPos) {
    if(any(lessThan(globalVoxelPos, ivec3(0))) || any(greaterThanEqual(globalVoxelPos, gridSize * BRICK_SIZE)))
        return false;
    
    ivec3 brickCoord = globalVoxelPos / BRICK_SIZE;
    ivec3 localPos   = globalVoxelPos % BRICK_SIZE;

    uint brickIndex = getBrickIndex(brickCoord);
    if (brickIndex == 0xFFFFFFFFu)
        return false;
    
    uint voxelIndex = getVoxelIndex(localPos);
    return isVoxelSolid(brickIndex, voxelIndex);
}

uint getMaterialIndex(uint brickIndex, uint voxelIndex) {
    Brick brick = bricks[brickIndex];
    uint count = 0;

    for (int i = 0; i <= voxelIndex / 64; i++) {
        uint64_t word = brick.bitmask[i];

        // Mask out bits after the voxel index in the relevant word
        if (i == voxelIndex / 64) {
            uint64_t mask = (1UL << (voxelIndex % 64)) - 1;
            word &= mask;
        }

        // Count the set bits in the word
        uvec2 unpacked = unpackUint2x32(word);
        count += bitCount(unpacked.x) + bitCount(unpacked.y);
    }

    return brick.materialOffset + count;
}

vec3 estimateNormal(ivec3 voxelPos) {
    vec3 normal = vec3(0.0);
    
    // Check adjacent voxels for normal estimation
    bool solidX1 = isVoxelSolidGlobal(voxelPos + ivec3(1, 0, 0));
    bool solidX0 = isVoxelSolidGlobal(voxelPos + ivec3(-1, 0, 0));
    bool solidY1 = isVoxelSolidGlobal(voxelPos + ivec3(0, 1, 0));
    bool solidY0 = isVoxelSolidGlobal(voxelPos + ivec3(0, -1, 0));
    bool solidZ1 = isVoxelSolidGlobal(voxelPos + ivec3(0, 0, 1));
    bool solidZ0 = isVoxelSolidGlobal(voxelPos + ivec3(0, 0, -1));
    
    normal.x = float(solidX0) - float(solidX1);
    normal.y = float(solidY0) - float(solidY1);
    normal.z = float(solidZ0) - float(solidZ1);

    return normalize(normal);
}

// PBR functions for specular reflections
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

HitInfo traceBrick(vec3 ro, vec3 rd, vec3 originalRo, uint brickIndex, float totalDist, ivec3 brickPos, float maxDist) {
    ro = clamp(ro, vec3(1e-6), vec3(float(BRICK_SIZE) - 1e-6));
    ivec3 voxel = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));

    const float BIG = 1e30;

    // Standard Amanatides & Woo tMax/tDelta in world-distance units
    vec3 tMax;
    vec3 tDelta;

    // X
    if (stp.x > 0) {
        tMax.x = (float(voxel.x) + 1.0 - ro.x) / rd.x;
        tDelta.x = 1.0 / rd.x;
    } else if (stp.x < 0) {
        tMax.x = (ro.x - float(voxel.x)) / (-rd.x);
        tDelta.x = 1.0 / (-rd.x);
    } else {
        tMax.x = BIG; tDelta.x = BIG;
    }
    // Y
    if (stp.y > 0) {
        tMax.y = (float(voxel.y) + 1.0 - ro.y) / rd.y;
        tDelta.y = 1.0 / rd.y;
    } else if (stp.y < 0) {
        tMax.y = (ro.y - float(voxel.y)) / (-rd.y);
        tDelta.y = 1.0 / (-rd.y);
    } else {
        tMax.y = BIG; tDelta.y = BIG;
    }
    // Z
    if (stp.z > 0) {
        tMax.z = (float(voxel.z) + 1.0 - ro.z) / rd.z;
        tDelta.z = 1.0 / rd.z;
    } else if (stp.z < 0) {
        tMax.z = (ro.z - float(voxel.z)) / (-rd.z);
        tDelta.z = 1.0 / (-rd.z);
    } else {
        tMax.z = BIG; tDelta.z = BIG;
    }

    float thisTotalDist = totalDist;

    int iterations = 0;
    const int MAX_BRICK_ITERATIONS = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE; // Worst case
    
    while (voxel.x <= BRICK_SIZE - 1 && voxel.x >= 0 && voxel.y <= BRICK_SIZE - 1 && voxel.y >= 0 && voxel.z <= BRICK_SIZE - 1 && voxel.z >= 0) {
        iterations++;
        if (iterations > MAX_BRICK_ITERATIONS) break; // Safety break

        if (thisTotalDist > maxDist) break;
        uint voxelIndex = getVoxelIndex(voxel);

        if (isVoxelSolid(brickIndex, voxelIndex)) {
            // Exact boundary hit and normal
            vec3 voxelMin = vec3(voxel);
            vec3 voxelMax = vec3(voxel) + 1.0;

            float tEntryX = rd.x != 0.0 ? ((stp.x > 0 ? voxelMin.x : voxelMax.x) - ro.x) / rd.x : -1e30;
            float tEntryY = rd.y != 0.0 ? ((stp.y > 0 ? voxelMin.y : voxelMax.y) - ro.y) / rd.y : -1e30;
            float tEntryZ = rd.z != 0.0 ? ((stp.z > 0 ? voxelMin.z : voxelMax.z) - ro.z) / rd.z : -1e30;
            float tEntry = max(tEntryX, max(tEntryY, tEntryZ));

            vec3 n = vec3(0.0);
            if (tEntry == tEntryX) n = vec3(-sign(rd.x), 0.0, 0.0);
            else if (tEntry == tEntryY) n = vec3(0.0, -sign(rd.y), 0.0);
            else n = vec3(0.0, 0.0, -sign(rd.z));

            vec3 brickLocalHit = ro + rd * tEntry;
            vec3 worldHitPos = vec3(brickPos) * float(BRICK_SIZE) * voxelScale + brickLocalHit * voxelScale;
            
            return HitInfo(true, worldHitPos, voxel + brickPos * BRICK_SIZE, n);
        }

        ivec3 oldVoxel = voxel;

        float minT = min(tMax.x, min(tMax.y, tMax.z));
        bvec3 stepAxes = lessThanEqual(tMax, vec3(minT + 1e-6));
        if (stepAxes.x) { voxel.x += stp.x; tMax.x += tDelta.x; }
        if (stepAxes.y) { voxel.y += stp.y; tMax.y += tDelta.y; }
        if (stepAxes.z) { voxel.z += stp.z; tMax.z += tDelta.z; }
        thisTotalDist = totalDist + minT;

        if (voxel == oldVoxel) break;
    }
    
    return HitInfo(false, vec3(0), ivec3(0), vec3(0));
}

HitInfo traceWorld(vec3 ro, vec3 rd, float maxDist) {
    ivec3 brick = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));

    const float BIG = 1e30;

    // Standard Amanatides & Woo for bricks
    vec3 tMax;
    vec3 tDelta;

    if (stp.x > 0) { tMax.x = (float(brick.x) + 1.0 - ro.x) / rd.x; tDelta.x = 1.0 / rd.x; }
    else if (stp.x < 0) { tMax.x = (ro.x - float(brick.x)) / (-rd.x); tDelta.x = 1.0 / (-rd.x); }
    else { tMax.x = BIG; tDelta.x = BIG; }

    if (stp.y > 0) { tMax.y = (float(brick.y) + 1.0 - ro.y) / rd.y; tDelta.y = 1.0 / rd.y; }
    else if (stp.y < 0) { tMax.y = (ro.y - float(brick.y)) / (-rd.y); tDelta.y = 1.0 / (-rd.y); }
    else { tMax.y = BIG; tDelta.y = BIG; }

    if (stp.z > 0) { tMax.z = (float(brick.z) + 1.0 - ro.z) / rd.z; tDelta.z = 1.0 / rd.z; }
    else if (stp.z < 0) { tMax.z = (ro.z - float(brick.z)) / (-rd.z); tDelta.z = 1.0 / (-rd.z); }
    else { tMax.z = BIG; tDelta.z = BIG; }
    
    float totalDist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        uint brickIndex = getBrickIndex(brick);
        if ((any(lessThan(brick, ivec3(0))) || any(greaterThanEqual(brick, gridSize)))) break;
        if (totalDist >= maxDist) break;

        if (brickIndex != 0xFFFFFFFFu) {
            // Robust brick entry via AABB
            float bEnter, bExit;
            vec3 bMin = vec3(brick);
            vec3 bMax = bMin + 1.0;
            if (intersectAABB(ro, rd, bMin, bMax, bEnter, bExit)) {
                float tStart = max(bEnter, 0.0);
                vec3 hitPos = ro + rd * tStart;
                vec3 uv3d = hitPos - bMin; // [0,1]
                uv3d = clamp(uv3d, vec3(1e-6), vec3(1.0) - vec3(1e-6));

                vec3 rdVoxel = rd * float(BRICK_SIZE);
                HitInfo hit = traceBrick(uv3d * float(BRICK_SIZE), rdVoxel, ro, brickIndex, totalDist, brick, maxDist);
                if (hit.hit) return hit;
            }
        }

        // Advance to next brick face; step across all tied axes
        vec3 oldTMax = tMax;
        float minT = min(tMax.x, min(tMax.y, tMax.z));
        bvec3 doStep = lessThanEqual(tMax, vec3(minT + 1e-6));
        if (doStep.x) { brick.x += stp.x; tMax.x += tDelta.x; }
        if (doStep.y) { brick.y += stp.y; tMax.y += tDelta.y; }
        if (doStep.z) { brick.z += stp.z; tMax.z += tDelta.z; }
        totalDist = minT;

        if (all(equal(tMax, oldTMax))) break;
    }
    
    return HitInfo(false, vec3(0), ivec3(0), vec3(0));
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 ndc = uv * 2.0 - 1.0;

    vec4 rayStartH = invViewProj * vec4(ndc, 0.0, 1.0);
    vec4 rayEndH   = invViewProj * vec4(ndc, 1.0, 1.0);

    // World-space origin and direction
    vec3 roW = cameraPos;
    vec3 rdW = normalize((rayEndH.xyz / max(rayEndH.w, 1e-6)) - (rayStartH.xyz / max(rayStartH.w, 1e-6)));

    float tEnter, tExit;
    HitInfo hit;

    vec3 epsilonVec = vec3(1e-3);
    vec3 worldSize = vec3(gridSize) * float(BRICK_SIZE) * voxelScale;
    if (!intersectAABB(roW, rdW, epsilonVec, worldSize - epsilonVec, tEnter, tExit)) {
        discard;
        return;
    }

    roW += rdW * max(tEnter, 0.0);
    float maxDistW = tEnter > 0.0 ? tExit - tEnter : tExit;

    // Convert to brick space for traversal (bricks per world unit)
    vec3 roB = roW * voxelScale / float(BRICK_SIZE);
    vec3 rdB = rdW * voxelScale / float(BRICK_SIZE);

    hit = traceWorld(roB, rdB, maxDistW);

    if (hit.hit) {
        vec3 lightDir = normalize(lightPos - hit.position);
        vec3 normal = normalize(hit.normal);
        // vec3 normal = estimateNormal(hit.voxelPos);

        // Shadows (optional). Keep modest bias; this is not trying to hide seams.
        bool inShadow = false;
        float distToCamera = length(hit.position - cameraPos);
        if (distToCamera < 300.0) {
            float biasN = max(0.5 * voxelScale, 0.01 * distToCamera);
            float biasL = 0.5 * voxelScale;
            vec3 shadowRoW = hit.position + normal * biasN + lightDir * biasL;
            float maxShadowDistW = length(lightPos - hit.position);
            vec3 shadowRoB = shadowRoW * voxelScale / float(BRICK_SIZE);
            vec3 lightDirB = lightDir * voxelScale / float(BRICK_SIZE);
            HitInfo sh = traceWorld(shadowRoB, lightDirB, maxShadowDistW);
            inShadow = sh.hit;
        }

        if (distToCamera > 100.0) normal = estimateNormal(hit.voxelPos);

        // Material
        ivec3 hitBrick = hit.voxelPos / BRICK_SIZE;
        ivec3 localVoxel = hit.voxelPos % BRICK_SIZE;
        uint brickIndex = getBrickIndex(hitBrick);
        uint voxelIndex = getVoxelIndex(localVoxel);
        MaterialInfo mat = materialInfos[int(materials[getMaterialIndex(brickIndex, voxelIndex)])];
        vec4 baseColor = mat.albedo;

        // PBR lighting
        vec3 V = normalize(cameraPos - hit.position);
        vec3 H = normalize(lightDir + V);
        float NdotL = max(dot(normal, lightDir), 0.0);
        float NdotV = max(dot(normal, V), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        vec3 F0 = mix(vec3(0.04), baseColor.rgb, mat.metallic);

        vec3 specular = vec3(0.0);
        if (distToCamera < 400.0) {
            float NDF = distributionGGX(normal, H, mat.roughness);
            float G = geometrySmith(normal, V, lightDir, mat.roughness);
            vec3 F = fresnelSchlick(HdotV, F0);
            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * NdotV * NdotL + 0.0001;
            specular = numerator / denominator;
        } else {
            specular = F0 * pow(max(dot(normal, H), 0.0), 16.0);
        }

        vec3 kS = specular;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - mat.metallic);
        vec3 ambient = baseColor.rgb * 0.1;
        vec3 finalColor = ambient;
        if (!inShadow) {
            vec3 diffuse = kD * baseColor.rgb / PI;
            vec3 lighting = (diffuse + specular) * lightColor * NdotL * lightIntensity;
            finalColor += lighting;
        }
        outColor = vec4(finalColor, baseColor.a);
    } else {
        discard;
    }
}
