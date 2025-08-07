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

vec3 lightPos = vec3(sin(time) * 100.0, 80.0, cos(time) * 100.0); // World-space directional light
vec3 lightColor = vec3(1.0);                    // White light
float lightIntensity = 10.0;

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

layout(std430, binding = 3) buffer MateriInfosBuffer {
    MaterialInfo materialInfos[];
};

const int MAX_STEPS = 200;
const float MAX_DIST = 250.0;

struct HitInfo {
    bool hit;
    vec3 position;
    ivec3 voxelPos;
};

bool intersectAABB(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax, out float tEnter, out float tExit) {
    vec3 tMin = (boxMin - ro) / rd;
    vec3 tMax = (boxMax - ro) / rd;

    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    tEnter = max(max(t1.x, t1.y), t1.z);
    tExit  = min(min(t2.x, t2.y), t2.z);

    return tExit >= max(tEnter, 0.0); // ray hits box and tExit is ahead of start
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
    if(any(lessThan(globalVoxelPos, ivec3(0))) || any(greaterThan(globalVoxelPos, gridSize * BRICK_SIZE)))
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
    
    for (int axis = 0; axis < 3; axis++) {
        ivec3 offset = ivec3(0);
        offset[axis] = 1;

        bool solidPos = isVoxelSolidGlobal(voxelPos + offset);
        bool solidNeg = isVoxelSolidGlobal(voxelPos - offset);

        normal[axis] = float(solidNeg) - float(solidPos); // inward - outward
    }

    return normalize(normal);
}

HitInfo traceBrick(vec3 ro, vec3 rd, vec3 originalRo, uint brickIndex, float totalDist, ivec3 brickPos, float maxDist) {
    ro = clamp(ro, vec3(1e-6), vec3(float(BRICK_SIZE) - 1e-6));
    ivec3 voxel = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));

    const float epsilon = 1e-6;
    vec3 safeRd = rd;
    if (abs(safeRd.x) < epsilon) safeRd.x = epsilon;
    if (abs(safeRd.y) < epsilon) safeRd.y = epsilon;
    if (abs(safeRd.z) < epsilon) safeRd.z = epsilon;

    vec3 deltaDist = 1.0 / safeRd;
    vec3 tMax = ((vec3(voxel) - ro) + 0.5 + vec3(stp) * 0.5) * deltaDist;
    deltaDist = abs(deltaDist);

    float thisTotalDist = totalDist;

    int iterations = 0;
    const int MAX_BRICK_ITERATIONS = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE; // Worst case
    
    while (voxel.x <= BRICK_SIZE - 1 && voxel.x >= 0 && voxel.y <= BRICK_SIZE - 1 && voxel.y >= 0 && voxel.z <= BRICK_SIZE - 1 && voxel.z >= 0) {
        iterations++;
        if (iterations > MAX_BRICK_ITERATIONS) break; // Safety break

        if (thisTotalDist > maxDist) break;
        uint voxelIndex = getVoxelIndex(voxel);

        if (isVoxelSolid(brickIndex, voxelIndex)) {
            // Calculate exact world hit position using ray marching
            // We need to find the exact point where the ray intersects the voxel surface
            
            // Calculate the exact intersection with the voxel boundaries
            vec3 voxelMin = vec3(voxel);
            vec3 voxelMax = vec3(voxel) + 1.0;
            
            // Find intersection with voxel faces
            vec3 t1 = (voxelMin - ro) / safeRd;
            vec3 t2 = (voxelMax - ro) / safeRd;
            vec3 tNear = min(t1, t2);
            vec3 tFar = max(t1, t2);
            
            float tEntry = max(max(tNear.x, tNear.y), tNear.z);
            
            // Convert brick-local hit position to world coordinates
            vec3 brickLocalHit = ro + safeRd * tEntry;
            vec3 worldHitPos = vec3(brickPos) * float(BRICK_SIZE) + brickLocalHit;
            
            return HitInfo(true, worldHitPos, voxel + brickPos * BRICK_SIZE);
        }

        ivec3 oldVoxel = voxel;
        
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            voxel.x += stp.x;
            tMax.x += deltaDist.x;
            thisTotalDist = totalDist + tMax.x;
        } else if (tMax.y < tMax.z) {
            voxel.y += stp.y;
            tMax.y += deltaDist.y;
            thisTotalDist = totalDist + tMax.y;
        } else {
            voxel.z += stp.z;
            tMax.z += deltaDist.z;
            thisTotalDist = totalDist + tMax.z;
        }

        if (voxel == oldVoxel) {
            break; // Stuck in same voxel, exit
        }
    }
    
    return HitInfo(false, vec3(0), ivec3(0));
}

HitInfo traceWorld(vec3 ro, vec3 rd, float maxDist) {
    const float epsilon = 1e-6;
    if (abs(rd.x) < epsilon) rd.x = rd.x >= 0.0 ? epsilon : -epsilon;
    if (abs(rd.y) < epsilon) rd.y = rd.y >= 0.0 ? epsilon : -epsilon;  
    if (abs(rd.z) < epsilon) rd.z = rd.z >= 0.0 ? epsilon : -epsilon;

    ivec3 brick = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));
    vec3 deltaDist = 1.0 / rd;
    vec3 tMax = ((vec3(brick) - ro) + 0.5 + vec3(stp) * 0.5) * deltaDist;
    deltaDist = abs(deltaDist);
    
    float totalDist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        uint brickIndex = getBrickIndex(brick);

        if ((any(lessThan(brick, ivec3(0))) || any(greaterThan(brick, gridSize)))) break;

        if (totalDist * float(BRICK_SIZE) >= maxDist) break;

        if (brickIndex != 0xFFFFFFFFu) {
            vec3 mini = ((vec3(brick) - ro) + 0.5 - 0.5 * vec3(stp)) * (1.0 / rd);
            float d = max(mini.x, max(mini.y, mini.z));
            vec3 intersect = ro + rd * d;
            vec3 uv3d = intersect - vec3(brick);

            if (brick == ivec3(floor(ro))) // Handle edge case where camera origin is inside of block
                uv3d = ro - vec3(brick);

            // Clamp uv3d to ensure it's within [0, 1] to avoid precision issues
            uv3d = clamp(uv3d, vec3(0.0), vec3(1.0));

            HitInfo hit = traceBrick(uv3d * float(BRICK_SIZE), rd, ro, brickIndex, totalDist * float(BRICK_SIZE), brick, maxDist);

            if (hit.hit)
                return hit;
        }

        vec3 oldTMax = tMax;

        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            brick.x += stp.x;
            tMax.x += deltaDist.x;
            totalDist = tMax.x;
        } else if (tMax.y < tMax.z) {
            brick.y += stp.y;
            tMax.y += deltaDist.y;
            totalDist = tMax.y;
        } else {
            brick.z += stp.z;
            tMax.z += deltaDist.z;
            totalDist = tMax.z;
        }

        if (length(tMax - oldTMax) < 1e-7) {
            break; // Not making progress, exit
        }
    }
    
    return HitInfo(false, vec3(0), ivec3(0));
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 ndc = uv * 2.0 - 1.0;

    vec4 rayStartH = invViewProj * vec4(ndc, 0.0, 1.0);
    vec4 rayEndH   = invViewProj * vec4(ndc, 1.0, 1.0);

    vec3 ro = cameraPos;
    vec3 rd = normalize((rayEndH.xyz / max(rayEndH.w, 1e-6)) - (rayStartH.xyz / max(rayStartH.w, 1e-6)));

    float tEnter, tExit;
    HitInfo hit;

    vec3 epsilonVec = vec3(1e-3);
    if (!intersectAABB(ro, rd, epsilonVec, vec3(gridSize) * float(BRICK_SIZE) - epsilonVec, tEnter, tExit)) {
        hit = HitInfo(false, vec3(0), ivec3(0));
    } else {
        ro += rd * max(tEnter, 0.0);
        float maxDist = tEnter > 0.0 ? tExit - tEnter : tExit;
        hit = traceWorld(ro / float(BRICK_SIZE), rd, 1000.0);
    }

    if (hit.hit) {
        vec3 lightDir = normalize(lightPos - hit.position);
        vec3 normal = estimateNormal(hit.voxelPos);

        // Offset ray origin to avoid self-shadowing
        // Move along light direction instead of surface normal
        float bias = 1.5; // Small bias along light direction
        vec3 shadowRo = hit.position + lightDir * bias;

        // Calculate shadow ray distance
        float maxShadowDist = length(lightPos - hit.position);
        
        // Use same coordinate space as primary ray (world space)
        HitInfo shadowHit = traceWorld(shadowRo / float(BRICK_SIZE), lightDir, maxShadowDist);

        // Fetch material properties
        uint brickIndex = getBrickIndex(ivec3(floor(hit.voxelPos / BRICK_SIZE)));
        uint voxelIndex = getVoxelIndex(hit.voxelPos % BRICK_SIZE);
        MaterialInfo mat = materialInfos[materials[getMaterialIndex(brickIndex, voxelIndex)]];
        vec4 baseColor = mat.albedo;

        // Lighting calculations
        float NdotL = max(dot(normal, lightDir), 0.0);
        
        // Start with ambient lighting
        vec3 ambient = baseColor.rgb * 0.2;
        vec3 finalColor = ambient;
        
        // Add diffuse lighting if NOT in shadow
        if (!shadowHit.hit) {
            vec3 diffuse = baseColor.rgb * lightColor * NdotL;
            finalColor += diffuse;
        }

        outColor = vec4(finalColor, baseColor.a);
    } else {
        outColor = background;
    }
}
