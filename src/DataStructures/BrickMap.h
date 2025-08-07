#ifndef BRICKMAP_H
#define BRICKMAP_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <array>
#include <memory.h>

#define BRICK_SIZE 8
#define VOXELS_PER_BRICK (BRICK_SIZE * BRICK_SIZE * BRICK_SIZE)

enum Material {
    AIR = 0,
    GRASS = 1,
    STONE = 2
};

struct alignas(16) MaterialInfo {
    glm::vec4 albedo;
    float metallic;
    float roughness;
    float padding = 0.0f;
};

/// @brief Individual Voxel struct for every Voxel for easy editing on the CPU. (might change to save storage space)
struct Voxel {
    Material material;
};

struct Brick {
    std::array<std::array<std::array<Voxel, BRICK_SIZE>, BRICK_SIZE>, BRICK_SIZE> voxels;
    glm::ivec3 pos;
};

/// @brief The brick in the format that is sent to the GPU.
struct GPUBrick {
    uint64_t bitmask[VOXELS_PER_BRICK / 64];
    uint32_t materialOffset;
};

/// @brief holds all the voxel data that is sent to the GPU.
struct GPUBrickMap {
    std::vector<GPUBrick> bricks;
    std::vector<uint32_t> indexData;
    std::vector<uint32_t> materials;
};

class BrickMap {
    std::vector<Brick> bricks;

    glm::ivec3 dimensions;

    struct hash_ivec3 {
        size_t operator()(const glm::ivec3& v) const {
            // 3 large prime numbers
            const size_t h1 = std::hash<int>{}(v.x * 73856093);
            const size_t h2 = std::hash<int>{}(v.y * 19349663);
            const size_t h3 = std::hash<int>{}(v.z * 83492791);
            return h1 ^ h2 ^ h3;
        }
    };

    std::unordered_map<glm::ivec3, int, hash_ivec3> indexTable;

public:
    BrickMap(glm::ivec3 dim) {
        dimensions = dim;
    }

    Brick& getBrick(glm::ivec3 pos) {
        if (indexTable.find(pos) != indexTable.end()) return bricks[indexTable[pos]];

        Brick brick{.pos = pos};
        int index = bricks.size();
        bricks.push_back(brick);
        indexTable.insert({pos, index});
        return bricks[index];
    }

    void fillBrick(glm::ivec3 pos, Material material) {
        Brick& brick = getBrick(pos);
        
        for (int x = 0; x < BRICK_SIZE; x++) {
            for (int y = 0; y < BRICK_SIZE; y++) {
                for (int z = 0; z < BRICK_SIZE; z++) {
                    brick.voxels[x][y][z].material = material;
                }
            }
        }
    }

    void setBrick(glm::ivec3 pos, const Brick& brick) {
        getBrick(pos) = brick;
    }

    void setVoxel(glm::ivec3 pos, Material material) {
        glm::ivec3 brickPos = pos / BRICK_SIZE;
        glm::ivec3 relVoxelPos = pos % BRICK_SIZE;
        getBrick(brickPos).voxels[relVoxelPos.x][relVoxelPos.y][relVoxelPos.z].material = material;
    }

    int getSize() {
        return bricks.size();
    }

    std::vector<Brick>& getBricks() {
        return bricks;
    }

    GPUBrickMap getGPUMap() {
    GPUBrickMap ret{};

    ret.bricks.resize(bricks.size());
    ret.indexData.resize(dimensions.x * dimensions.y * dimensions.z, 0xFFFFFFFF);

    for (auto& brick : bricks) {
        GPUBrick gpuBrick{};
        // Initialize bitmask to zero
        memset(gpuBrick.bitmask, 0, sizeof(gpuBrick.bitmask));
        
        gpuBrick.materialOffset = ret.materials.size();
        ret.indexData[brick.pos.x + brick.pos.y * dimensions.x + brick.pos.z * dimensions.x * dimensions.y] = ret.bricks.size();
        
        for (int z = 0; z < BRICK_SIZE; z++) {
            for (int y = 0; y < BRICK_SIZE; y++) {
                for (int x = 0; x < BRICK_SIZE; x++) {
                    if (brick.voxels[x][y][z].material != AIR) {
                        // CORRECT voxel index calculation (matches your shader!)
                        uint32_t voxelIndex = x + y * BRICK_SIZE + z * BRICK_SIZE * BRICK_SIZE;
                        uint32_t wordIndex = voxelIndex / 64;
                        uint32_t bitIndex = voxelIndex % 64;
                        
                        gpuBrick.bitmask[wordIndex] |= 1ULL << bitIndex;
                        ret.materials.push_back(brick.voxels[x][y][z].material);
                    }
                }
            }
        }
        ret.bricks.push_back(gpuBrick);
    }

    return ret;
}

    int getSizeInBytes() {
        int brickSize = bricks.size() * sizeof(Brick);
        int indexSize = indexTable.size() * (sizeof(glm::ivec3) + sizeof(int));
        return brickSize + indexSize + sizeof(BrickMap);
    }
};

#endif // BRICKMAP_H