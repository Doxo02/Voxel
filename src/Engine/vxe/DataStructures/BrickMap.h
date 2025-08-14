#ifndef BRICKMAP_H
#define BRICKMAP_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <array>

#include <FastNoiseLite.h>
#include <mutex>

#include "Grid.h"

#define BRICK_SIZE 8
#define VOXELS_PER_BRICK (BRICK_SIZE * BRICK_SIZE * BRICK_SIZE)

// TODO: extract Material stuff

namespace vxe {
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
    struct GPUGrid {
        std::vector<GPUBrick> bricks;
        std::vector<uint32_t> indexData;
        std::vector<uint32_t> materials;
    };

    class BrickMap : public Grid {
        std::vector<Brick> bricks;

        glm::ivec3 dimensions;

        FastNoiseLite m_noise;

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

        mutable std::mutex m_chunkGenMutex;

    public:
        explicit BrickMap(glm::ivec3 dim);
        ~BrickMap() override = default ;
        Brick& getBrick(glm::ivec3 pos);

        void fillBrick(glm::ivec3 pos, Material material);
        void setBrick(glm::ivec3 pos, const Brick& brick);
        void setVoxel(glm::ivec3 pos, Material material) override;
        void fillRegion(glm::ivec3 position, glm::ivec3 extents, Material material) override;
        bool generateChunk(const glm::ivec3& pos) override;

        std::vector<Brick>& getBricks();
        GPUGrid getGPUGrid() override;

        size_t getSize() override;
        size_t getSizeInBytes() override;
    };
}

#endif // BRICKMAP_H