#ifndef BRICKMAP_H
#define BRICKMAP_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <array>

#define BRICK_SIZE 8
#define BRICK_TOTAL 512

struct Voxel {
    glm::vec4 color = glm::vec4(0);
};

struct BrickMap {
    std::array<std::array<std::array<Voxel, BRICK_SIZE>, BRICK_SIZE>, BRICK_SIZE> voxels;
    glm::ivec3 pos;
};

struct GPUBrickMap {
    uint64_t bitmask[8];
    uint32_t colorOffset;
};

struct GPUBrickGrid {
    std::vector<GPUBrickMap> bricks;
    std::vector<glm::ivec3> positions;
    std::vector<glm::vec4> colors;
};

class BrickGrid {
    std::vector<BrickMap> bricks;

    glm::ivec3 dimensions;

    struct hash_ivec3 {
        size_t operator()(const glm::ivec3& v) const {
            return std::hash<int>()(v.x) ^ std::hash<int>()(v.y << 1) ^ std::hash<int>()(v.z << 2);
        }
    };

    std::unordered_map<glm::ivec3, int, hash_ivec3> indexTable;

public:
    BrickGrid(glm::ivec3 dim) {
        dimensions = dim;
    }

    BrickMap& getBrick(glm::ivec3 pos) {
        if (indexTable.find(pos) != indexTable.end()) return bricks[indexTable[pos]];

        BrickMap brick{.pos = pos};
        int index = bricks.size();
        bricks.push_back(brick);
        indexTable.insert({pos, index});
    }

    void fillBrick(glm::ivec3 pos, glm::vec4 color) {
        BrickMap& brick = getBrick(pos);
        
        for (int x = 0; x < BRICK_SIZE; x++) {
            for (int y = 0; y < BRICK_SIZE; y++) {
                for (int z = 0; z < BRICK_SIZE; z++) {
                    brick.voxels[x][y][z].color = color;
                }
            }
        }
    }

    void setBrick(glm::ivec3 pos, const BrickMap& brick) {
        getBrick(pos) = brick;
    }

    void setVoxel(glm::ivec3 pos, glm::vec4 color) {
        glm::ivec3 brickPos = pos / BRICK_SIZE;
        glm::ivec3 relVoxelPos = pos % BRICK_SIZE;
        getBrick(brickPos).voxels[relVoxelPos.x][relVoxelPos.y][relVoxelPos.z].color = color;
    }

    int getSize() {
        return bricks.size();
    }

    std::vector<BrickMap>& getBricks() {
        return bricks;
    }

    GPUBrickGrid getGPUGrid() {
        GPUBrickGrid ret{};

        ret.bricks.resize(bricks.size());

        for (auto& brick : bricks) {
            GPUBrickMap gpuBrick{};
            gpuBrick.colorOffset = ret.colors.size();
            ret.positions.push_back(brick.pos);
            for (int z = 0; z < BRICK_SIZE; z++) {
                for (int x = 0; x < BRICK_SIZE; x++) {
                    for (int y = 0; y < BRICK_SIZE; y++) {
                        if (brick.voxels[x][y][z].color != glm::vec4(0)) {
                            gpuBrick.bitmask[z] |= 1 << x + y * BRICK_SIZE;
                            ret.colors.push_back(brick.voxels[x][y][z].color);
                        }
                    }
                }
            }
            ret.bricks.push_back(gpuBrick);
        }

        return ret;
    }
};

#endif // BRICKMAP_H