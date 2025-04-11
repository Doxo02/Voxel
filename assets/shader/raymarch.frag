// TODO: fix floating precission(?) issue
#version 450
#extension GL_ARB_gpu_shader_int64 : enable

#define BRICK_SIZE 8

layout(location = 0) out vec4 outColor;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform vec2 resolution;

uniform uint brickSize;
uniform ivec3 gridSize;

vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5)); // World-space directional light
vec3 lightColor = vec3(1.0);                    // White light

vec4 background = vec4(0.1, 0.1, 0.8, 1.0);

struct Brick {
    uint64_t bitmask[(BRICK_SIZE * BRICK_SIZE * BRICK_SIZE) / 64];
    uint colorOffset;
};

layout(std430, binding = 0) buffer BrickMapBuffer {
    uint brickMap[];
};

layout(std430, binding = 1) buffer BrickBuffer {
    Brick bricks[];
};

layout(std430, binding = 2) buffer ColorBuffer {
    vec4 colors[];
};

const int MAX_STEPS = 200;
const float MAX_DIST = 1000.0;

bool intersectAABB(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax, out float tEnter, out float tExit) {
    vec3 tMin = (boxMin - ro) / rd;
    vec3 tMax = (boxMax - ro) / rd;

    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    tEnter = max(max(t1.x, t1.y), t1.z);
    tExit  = min(min(t2.x, t2.y), t2.z);

    return tExit >= max(tEnter, 0.0); // ray hits box and tExit is ahead of start
}

int getVoxelIndex(ivec3 localPos) {
    return localPos.x + localPos.y * BRICK_SIZE + localPos.z * BRICK_SIZE * BRICK_SIZE;
}

bool isVoxelSolid(uint brickIndex, int voxelIndex) {
    Brick brick = bricks[brickIndex];
    uint64_t word = brick.bitmask[voxelIndex / 64];
    return ((word >> (voxelIndex % 64)) & 1UL) != 0UL;
}

uint getBrickIndex(ivec3 brickPos) {
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
    
    int voxelIndex = getVoxelIndex(localPos);
    return isVoxelSolid(brickIndex, voxelIndex);
}

uint getColorIndex(uint brickIndex, int voxelIndex) {
    Brick brick = bricks[brickIndex];
    uint count = 0;
    for (int i = 0; i < BRICK_SIZE; i++) {
        if (voxelIndex / 64 - i == 0) {
            uint64_t word = brick.bitmask[i] << (64 - (voxelIndex % 64));
            uvec2 mask = unpackUint2x32(word);
            uvec2 tmpCount = bitCount(mask);
            count += tmpCount.x + tmpCount.y;
            break;
        }
        uint64_t word = brick.bitmask[i];
        uvec2 mask = unpackUint2x32(word);
        uvec2 tmpCount = bitCount(mask);
        count += tmpCount.x + tmpCount.y;
    }
    return brick.colorOffset + count;
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

vec4 traceBrick(vec3 ro, vec3 rd, uint brickIndex, float totalDist, ivec3 brickPos) {
    ro = clamp(ro, vec3(0.0001), vec3(float(BRICK_SIZE) - 0.0001));
    ivec3 voxel = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));
    vec3 deltaDist = 1.0 / rd;
    vec3 tMax = ((vec3(voxel) - ro) + 0.5 + vec3(stp) * 0.5) * deltaDist;
    deltaDist = abs(deltaDist);

    float thisTotalDist = totalDist;
    
    while (voxel.x <= BRICK_SIZE - 1 && voxel.x >= 0 && voxel.y <= BRICK_SIZE - 1 && voxel.y >= 0 && voxel.z <= BRICK_SIZE - 1 && voxel.z >= 0) {
        int voxelIndex = getVoxelIndex(voxel);
        if (isVoxelSolid(brickIndex, voxelIndex)) {
            // return vec4(vec3(voxel) / float(BRICK_SIZE), 1.0);
            vec4 baseColor = colors[getColorIndex(brickIndex, voxelIndex)];
            vec3 normal = estimateNormal(voxel + brickPos * BRICK_SIZE);

            // return vec4(normal, 1.0);

            float NdotL = max(dot(normal, lightDir), 0.0);
            vec3 litColor = baseColor.rgb * lightColor * NdotL;

            vec3 ambient = 0.15 * baseColor.rgb;
            vec3 finalColor = ambient + litColor;

            // vec3 viewDir = normalize(cameraPos - (ro + rd * thisTotalDist)); // Approximate view direction
            // vec3 reflectDir = reflect(-lightDir, normal);
            // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            // finalColor += vec3(spec);

            return vec4(finalColor, baseColor.a);
        }
        
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
    }
    
    return vec4(0.0);
}

vec4 traceWorld(vec3 ro, vec3 rd) {    
    float tEnter, tExit;
    if (!intersectAABB(ro, rd, vec3(0.0), vec3(gridSize), tEnter, tExit)) {
        return background;
    }

    ro = ro + rd * max(tEnter, 0.0);

    ivec3 brick = ivec3(floor(ro + 1e-4));
    ivec3 stp = ivec3(sign(rd));
    vec3 deltaDist = 1.0 / rd;
    vec3 tMax = ((vec3(brick) - ro) + 0.5 + vec3(stp) * 0.5) * deltaDist;
    deltaDist = abs(deltaDist);
    
    float totalDist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        if (totalDist > (MAX_DIST / float(BRICK_SIZE))) break;

        uint brickIndex = getBrickIndex(brick);

        if ((any(lessThan(brick, ivec3(0))) || any(greaterThan(brick, gridSize)))) break;

        if (brickIndex != 0xFFFFFFFFu) {
            vec3 mini = ((vec3(brick) - ro) + 0.5 - 0.5 * vec3(stp)) * (1.0 / rd);
            float d = max(mini.x, max(mini.y, mini.z));
            vec3 intersect = ro + rd * d;
            vec3 uv3d = intersect - vec3(brick);

            if (brick == floor(ro)) // Handle edge case where camera origin is inside of block
                uv3d = ro - vec3(brick);

            vec4 hit = traceBrick(uv3d * float(BRICK_SIZE), rd, brickIndex, totalDist, brick);

            if (hit.a > 0.95) 
                    return hit;
        }

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
    }
    
    return background;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 ndc = uv * 2.0 - 1.0;

    vec4 rayStartH = invViewProj * vec4(ndc, 0.0, 1.0);
    vec4 rayEndH   = invViewProj * vec4(ndc, 1.0, 1.0);

    vec3 ro = cameraPos;
    vec3 rd = normalize((rayEndH.xyz / max(rayEndH.w, 1e-6)) - (rayStartH.xyz / max(rayStartH.w, 1e-6)));

    outColor = traceWorld(ro, rd);
}
