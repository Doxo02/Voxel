#include "BrickMap.h"

#include "../Events/Events.h"

#include <algorithm>

namespace vxe {
    BrickMap::BrickMap(const glm::ivec3& dimensions) : m_dimensions(dimensions), m_noise(0) {
        m_indexData.resize(dimensions.x * dimensions.y * dimensions.z, 0xFFFFFFFF);
        m_indexDataSSBO = ShaderStorageBuffer::create(0);
        m_bricksSSBO = ShaderStorageBuffer::create(1);
        m_materialDataSSBO = ShaderStorageBuffer::create(2);

        m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

    BrickMap::~BrickMap() {}

    void BrickMap::ensureOffsetsValid() {
        if (m_offsetsNeedRebuild) {
            rebuildMaterialOfssets();
            m_offsetsNeedRebuild = false;
        }
    }

    void BrickMap::setVoxel(glm::ivec3 position, Material mat) {
        int insertionPoint = setVoxelPrivate(position, mat);

        if (insertionPoint != -1)
            updateMaterialOffset(insertionPoint);
    }

    void BrickMap::fillRegion(glm::ivec3 position, glm::ivec3 extents, Material material) {
        for (int x = position.x; x < position.x + extents.x; x++) {
            for (int y = position.y; y < position.y + extents.y; y++) {
                for (int z = position.z; z < position.z + extents.z; z++) {
                    setVoxelPrivate(glm::ivec3(x, y, z), material);
                }
            }
        }

        markOffsetsInvalid();
        VXE_DISPATCH(GridChangedEvent);
    }

    bool BrickMap::generateChunk(const glm::ivec3& pos) {
        std::lock_guard lock(m_chunkGenMutex);

        if(pos.x >= m_dimensions.x || pos.y >= m_dimensions.y || pos.z >= m_dimensions.z ||
        pos.x < 0 || pos.y < 0 || pos.z < 0)
            return false;

        for (int z = 0; z < BRICK_SIZE; z++) {
            for (int x = 0; x < BRICK_SIZE; x++) {
                int yTop = (m_noise.GetNoise((float) (x + pos.x * BRICK_SIZE), (float) (z + pos.z * BRICK_SIZE)) + 1.0) / 2.0 * ((m_dimensions.y * BRICK_SIZE) / 4.0) + 1.0;

                if (yTop < pos.y * BRICK_SIZE) continue;

                if (yTop < pos.y * BRICK_SIZE + BRICK_SIZE)
                    setVoxelPrivate(glm::ivec3(x + pos.x * BRICK_SIZE, yTop, z + pos.z * BRICK_SIZE), Material::GRASS);

                int localYTop = std::min(yTop - pos.y * BRICK_SIZE, BRICK_SIZE);

                for (int y = 0; y < localYTop; y++) {
                    setVoxelPrivate(glm::ivec3(x + pos.x * BRICK_SIZE, y + pos.y * BRICK_SIZE, z + pos.z * BRICK_SIZE), Material::STONE);
                }
            }
        }

        markOffsetsInvalid();
        VXE_DISPATCH(GridChangedEvent);

        return true;
    }

    GPUGrid BrickMap::getGPUGrid() {
        return {0};
    }

    size_t BrickMap::getSize() {
        return m_bricks.size();
    }

    size_t BrickMap::getSizeInBytes() {
        size_t size = m_bricks.size() * sizeof(Brick);
        size += m_indexData.size() * sizeof(uint32_t);
        size += m_materialData.size() * sizeof(uint32_t);
        size += sizeof(m_bricks) + sizeof(m_indexData) + sizeof(m_materialData);
        return size;
    }

    void BrickMap::uploadToGPU() {
        ensureOffsetsValid();
        m_bricksSSBO->setData(m_bricks.data(), m_bricks.size() * sizeof(Brick));
        m_indexDataSSBO->setData(m_indexData.data(), m_indexData.size() * sizeof(uint32_t));
        m_materialDataSSBO->setData(m_materialData.data(), m_materialData.size() * sizeof(uint32_t));
    }

    void BrickMap::insertBrickInSortedOrder(size_t brickIndex) {
        uint32_t offset = m_bricks[brickIndex].materialOffset;

        auto it = std::lower_bound(m_bricksByOffset.begin(), m_bricksByOffset.end(), offset, [this](size_t idx, uint32_t offset) {
            return m_bricks[idx].materialOffset < offset;
        });

        m_bricksByOffset.insert(it, brickIndex);
    }

    void BrickMap::removeBrickFromSortedOrder(size_t brickIndex) {
        
    }

    void BrickMap::rebuildMaterialOfssets() {
        uint32_t currentOffset = 0;
        for (auto& brick : m_bricks) {
            brick.materialOffset = currentOffset;
            currentOffset += brick.getVoxelCount();
        }
    }

    int BrickMap::setVoxelPrivate(glm::ivec3 position, Material material) {
        glm::ivec3 brickPos = position / glm::ivec3(BRICK_SIZE);
        glm::ivec3 localVoxelPos = position % glm::ivec3(BRICK_SIZE);
        uint32_t voxelIndex = localVoxelPos.x + localVoxelPos.y * BRICK_SIZE + localVoxelPos.z * BRICK_SIZE * BRICK_SIZE;
        uint32_t wordIndex = voxelIndex / 64;
        uint32_t bitIndex = voxelIndex % 64;
        
        if (brickExists(brickPos)) {
            Brick& brick = getBrick(brickPos);

            bool voxelWasSet = (brick.bitmask[wordIndex] & (1UL << bitIndex)) != 0;

            uint32_t materialIndex = brick.materialOffset;

            for (uint32_t w = 0; w < wordIndex; w++) {
                materialIndex += __builtin_popcountl(brick.bitmask[w]);
            }
            materialIndex += __builtin_popcountl(brick.bitmask[wordIndex] & ((1UL << bitIndex) - 1));

            if (!voxelWasSet) {
                brick.bitmask[wordIndex] |= 1UL << bitIndex;

                m_materialData.insert(m_materialData.begin() + materialIndex, static_cast<uint32_t>(material));

                return materialIndex;
            } else {
                // if voxel was set before just replace the old material data
                m_materialData[materialIndex] = static_cast<uint32_t>(material);
            }
        } else {
            Brick& brick = createBrick(brickPos);

            brick.bitmask[wordIndex] |= 1UL << bitIndex;
            brick.materialOffset = m_materialData.size();
            m_materialData.push_back(static_cast<uint32_t>(material));
        }

        return -1;
    }

    bool BrickMap::brickExists(glm::ivec3 brickPos) {
        return m_indexData[brickPos.x + brickPos.y * m_dimensions.x + brickPos.z * m_dimensions.x * m_dimensions.y] != 0xFFFFFFFF;
    }

    Brick& BrickMap::getBrick(glm::ivec3 brickPos) {
        size_t index = m_indexData[brickPos.x + brickPos.y * m_dimensions.x + brickPos.z * m_dimensions.x * m_dimensions.y];
        // TODO: error checking

        return m_bricks[index];
    }

    Brick& BrickMap::createBrick(glm::ivec3 brickPos) {
        size_t index = m_bricks.size();
        m_bricks.push_back({0});
        m_indexData[brickPos.x + brickPos.y * m_dimensions.x + brickPos.z * m_dimensions.x * m_dimensions.y] = index;
        insertBrickInSortedOrder(index);
        return m_bricks[index];
    }

    void BrickMap::updateMaterialOffset(uint32_t insertionPoint) {
        // for (auto& brick : m_bricks) {
        //     if (brick.materialOffset >= insertionPoint) {
        //         brick.materialOffset++;
        //     }
        // }

        auto it = std::lower_bound(m_bricksByOffset.begin(), m_bricksByOffset.end(), insertionPoint, [this](size_t idx, uint32_t offset) {
            return m_bricks[idx].materialOffset < offset;
        });

        for (auto iter = it; iter != m_bricksByOffset.end(); iter++) {
            m_bricks[*iter].materialOffset++;
        }
    }
}