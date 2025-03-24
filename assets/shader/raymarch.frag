#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(location = 0) out vec4 outColor;

// Camera data
uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform vec2 resolution;

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
const int MAX_STEPS = 5000;

const vec3 gridSize = vec3(64, 32, 64);

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
    int count = 0;
    for (int i = 0; i < voxelIndex; ++i) {
        if (isVoxelSolid(brickIndex, i)) {
            count++;
        }
    }
    return brick.colorOffset + count;
}

// Main raymarch
vec4 raymarch(vec3 ro, vec3 rd) {
    vec4 finalColor = vec4(0.0);
    float t = 0.0;
    const float maxDist = 100.0;

    ivec3 lastVoxel = ivec3(-999);

    for (int step = 0; step < MAX_STEPS; ++step) {
        vec3 p = ro + rd * t;

        if (t > maxDist) break;

        ivec3 worldVoxel = ivec3(floor(p));

        if (worldVoxel == lastVoxel) {
            t += STEP_SIZE;
            continue;
        }

        lastVoxel = worldVoxel;

        ivec3 brickCoord = ivec3(floor(worldVoxel / 8.0));
        ivec3 localPos = worldVoxel - brickCoord * 8;
        if (any(lessThan(brickCoord, ivec3(0))) || any(greaterThanEqual(brickCoord, gridSize))) {
            break;
        }

        // Get brick index
        uint brickIndex = texelFetch(brickMap, brickCoord, 0).r;

        if (brickIndex != 0xFFFFFFFFu) {
            int voxelIndex = getVoxelIndex(localPos);

            if (isVoxelSolid(brickIndex, voxelIndex)) {
                uint colorIdx = getColorIndex(brickIndex, voxelIndex);
                vec4 voxelColor = colors[colorIdx];

                // vec3 debugColor = vec3(brickCoord) / vec3(gridSize);
                // return vec4(debugColor, 1.0f);

                // vec3 localColor = vec3(localPos) / 8;
                // return vec4(localColor, 1.0f);

                // Basic shading (e.g., eye light)
                finalColor = voxelColor;
                break;
            }
        }

        t += STEP_SIZE;
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
