#ifndef VXE_BRICKMAP_H
#define VXE_BRICKMAP_H

#include "Grid.h"

#include "../Rendering/graphics/ShaderStorageBuffer.h"

#include <FastNoiseLite.h>

#include <vector>
#include <mutex>

namespace vxe {
    static constexpr size_t BRICK_SIZE = 8;
    static constexpr size_t VOXELS_PER_BRICK = BRICK_SIZE * BRICK_SIZE * BRICK_SIZE;

    struct Brick {
        uint64_t bitmask[VOXELS_PER_BRICK / 64];
        uint32_t materialOffset;

        inline uint32_t getVoxelCount() {
            uint32_t count = 0;
            for (int i = 0; i < VOXELS_PER_BRICK / 64; i++) {
                count += __builtin_popcountl(bitmask[i]);
            }
            return count;
        }
    };

    struct GPUGrid {
        uint8_t dummy;
    };

    class BrickMap : public Grid {
        public:
            BrickMap(const glm::ivec3& dimensions);
            ~BrickMap();

            void setVoxel(glm::ivec3 position, Material material) override;
            void fillRegion(glm::ivec3 position, glm::ivec3 extents, Material material) override;
            bool generateChunk(const glm::ivec3& pos) override;

            void uploadToGPU() override;
            GPUGrid getGPUGrid() override;
            size_t getSize() override;
            size_t getSizeInBytes() override;
        
        private:
            glm::ivec3 m_dimensions;
            std::vector<Brick> m_bricks;
            std::vector<size_t> m_bricksByOffset;
            std::vector<uint32_t> m_indexData;
            std::vector<uint32_t> m_materialData;

            std::unique_ptr<ShaderStorageBuffer> m_bricksSSBO;
            std::unique_ptr<ShaderStorageBuffer> m_indexDataSSBO;
            std::unique_ptr<ShaderStorageBuffer> m_materialDataSSBO;

            FastNoiseLite m_noise;
            mutable std::mutex m_chunkGenMutex;

            bool m_offsetsNeedRebuild = false;

            void ensureOffsetsValid();
            void markOffsetsInvalid() { m_offsetsNeedRebuild = true; }

            void insertBrickInSortedOrder(size_t brickIndex);
            void removeBrickFromSortedOrder(size_t brickIndex);
            void rebuildMaterialOfssets();

            int setVoxelPrivate(glm::ivec3 position, Material material);

            bool brickExists(glm::ivec3 brickPos);
            Brick& getBrick(glm::ivec3 brickPos);
            Brick& createBrick(glm::ivec3 brickPos);
            void updateMaterialOffset(uint32_t insertionPoint);
    };
}

#endif