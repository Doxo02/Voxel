#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(location = 0) out vec4 outColor;

// Camera data
uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform vec2 resolution;

uniform uint brickSize;
uniform vec3 gridSize;

// Voxel world grid (brick index map)
layout(binding = 0) uniform usampler3D brickMap; // brick indices

struct Brick {
    uint64_t bitmask[8];
    uint colorOffset;
};

// Brick buffer (bitmask + color offset)
layout(std430, binding = 1) buffer BrickBuffer {
    Brick bricks[];
};

// Color buffer
layout(std430, binding = 2) buffer ColorBuffer {
    vec4 colors[];
};

const float STEP_SIZE = 0.05f; // Tune as needed
const int MAX_STEPS = 512;

// Util: compute linear voxel index inside a brick (0–511)
int getVoxelIndex(ivec3 localPos) {
    return localPos.x + localPos.y * 8 + localPos.z * 64;
}

// Util: check bitmask for voxel occupancy
bool isVoxelSolid(uint brickIndex, int voxelIndex) {
    Brick brick = bricks[brickIndex];
    uint64_t word = brick.bitmask[voxelIndex / 64];
    return ((word >> (voxelIndex % 64)) & 1UL) != 0UL;
}

// Util: count set bits before a given voxel in bitmask (slow version)
uint getColorIndex(uint brickIndex, int voxelIndex) {
    Brick brick = bricks[brickIndex];
    uint count = 0;
    for (int i = 0; i < brickSize; i++) {
        if (voxelIndex / 64 - i == 0) {
            uint64_t word = brick.bitmask[i] >> (voxelIndex % 64);
            uvec2 mask = unpackUint2x32(word);
            uvec2 tmpCount = bitCount(mask);
            count += tmpCount.x + tmpCount.y - 1;
            break;
        }
        uint64_t word = brick.bitmask[i];
        uvec2 mask = unpackUint2x32(word);
        uvec2 tmpCount = bitCount(mask);
        count += tmpCount.x + tmpCount.y;
    }
    return brick.colorOffset + count;
}

// Main raymarch
vec4 raymarch(vec3 ro, vec3 rd) {
    vec4 finalColor = vec4(0.0);
    const float maxDist = 100.0;

    ivec3 curVox = ivec3(floor(ro));
    vec3 s = sqrt(1 + rd * rd);
    ivec3 stp = ivec3(sign(rd));

    vec3 tMax = ro + s;

    float totalDist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        if (totalDist > maxDist) break;

        ivec3 brickCoord = curVox / int(brickSize);

        uint brickIndex = texelFetch(brickMap, brickCoord, 0).r;
        if (brickIndex != 0xFFFFFFFFu) {
            ivec3 localPos = curVox - brickCoord * 8;
            int voxelIndex = getVoxelIndex(curVox);

            if (isVoxelSolid(brickIndex, voxelIndex)) {
                finalColor = colors[getColorIndex(brickIndex, voxelIndex)];
                break;
            }
        }

        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            totalDist = tMax.x;
            tMax.x += s.x;
            curVox.x += stp.x;
        } else if (tMax.y < tMax.z) {
            totalDist = tMax.y;
            tMax.y += s.y;
            curVox.y += stp.y;
        } else {
            totalDist = tMax.z;
            tMax.z += s.z;
            curVox.z += stp.z;
        }
    }

    return finalColor;
}

// Entry point
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 ndc = uv * 2.0 - 1.0;

    vec4 rayStartH = invViewProj * vec4(ndc, 0.0, 1.0);
    vec4 rayEndH   = invViewProj * vec4(ndc, 1.0, 1.0);

    vec3 ro = cameraPos;
    vec3 rd = normalize((rayEndH.xyz / rayEndH.w) - (rayStartH.xyz / rayStartH.w));

    outColor = raymarch(ro, rd);
}
