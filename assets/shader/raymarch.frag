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
const int MAX_STEPS = 200;

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

vec4 traceBlock(vec3 ro, vec3 rd, uint brickIndex) {
    ro = clamp(ro, vec3(0.0001), vec3(7.9999));
    ivec3 voxel = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));
    vec3 deltaDist = 1.0 / rd;
    vec3 tMax = ((voxel - ro) + 0.5 + stp * 0.5) * deltaDist;
    deltaDist = abs(deltaDist);
    
    while (voxel.x <= 7.0 && voxel.x >= 0.0 && voxel.y <= 7.0 && voxel.y >= 0.0 && voxel.z <= 7.0 && voxel.z >= 0.0) {
        int voxelIndex = getVoxelIndex(voxel);
        if (isVoxelSolid(brickIndex, voxelIndex))
            return colors[getColorIndex(brickIndex, voxelIndex)];
        
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            voxel.x += stp.x;
            tMax.x += deltaDist.x;
        } else if (tMax.y < tMax.z) {
            voxel.y += stp.y;
            tMax.y += deltaDist.y;
        } else {
            voxel.z += stp.z;
            tMax.z += deltaDist.z;
        }
    }
    
    return vec4(0.0);
}

vec4 traceWorld(vec3 ro, vec3 rd) {    
    ivec3 brick = ivec3(floor(ro));
    ivec3 stp = ivec3(sign(rd));
    vec3 deltaDist = 1.0 / rd;
    vec3 tMax = ((brick-ro) + 0.5 + stp * 0.5) * deltaDist;
    deltaDist = abs(deltaDist);
    
    for (int i = 0; i < MAX_STEPS; i++) {
            uint brickIndex = texelFetch(brickMap, brick, 0).r;

        if (brickIndex != 0xFFFFFFFFu) {
            vec3 mini = ((brick - ro) + 0.5 - 0.5 * vec3(stp)) * (1.0 / rd);
            float d = max (mini.x, max (mini.y, mini.z));
            vec3 intersect = ro + rd*d;
            vec3 uv3d = intersect - brick;

            if (brick == floor(ro)) // Handle edge case where camera origin is inside of block
                uv3d = ro - brick;

            vec4 hit = traceBlock(uv3d * 8.0, rd, brickIndex);

            if (hit.a > 0.95) 
                    return hit;
        }
       
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            brick.x += stp.x;
            tMax.x += deltaDist.x;
        } else if (tMax.y < tMax.z) {
            brick.y += stp.y;
            tMax.y += deltaDist.y;
        } else {
            brick.z += stp.z;
            tMax.z += deltaDist.z;
        }
    }
    
    return vec4(0.0);
}

// Entry point
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 ndc = uv * 2.0 - 1.0;

    vec4 rayStartH = invViewProj * vec4(ndc, 0.0, 1.0);
    vec4 rayEndH   = invViewProj * vec4(ndc, 1.0, 1.0);

    vec3 ro = cameraPos;
    vec3 rd = normalize((rayEndH.xyz / max(rayEndH.w, 1e-6)) - (rayStartH.xyz / max(rayStartH.w, 1e-6)));

    outColor = traceWorld(ro, rd);
}
