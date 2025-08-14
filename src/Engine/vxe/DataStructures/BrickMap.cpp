#include "BrickMap.h"

#include <cstring>
#include <mutex>

vxe::BrickMap::BrickMap(const glm::ivec3 dim) : dimensions(dim), m_noise(0) {
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

vxe::Brick& vxe::BrickMap::getBrick(glm::ivec3 pos) {
    if (indexTable.find(pos) != indexTable.end()) return bricks[indexTable[pos]];

    Brick brick{.pos = pos};
    int index = bricks.size();
    bricks.push_back(brick);
    indexTable.insert({pos, index});
    return bricks[index];
}

void vxe::BrickMap::fillBrick(glm::ivec3 pos, Material material) {
    Brick& brick = getBrick(pos);
    
    for (int x = 0; x < BRICK_SIZE; x++) {
        for (int y = 0; y < BRICK_SIZE; y++) {
            for (int z = 0; z < BRICK_SIZE; z++) {
                brick.voxels[x][y][z].material = material;
            }
        }
    }
}

void vxe::BrickMap::setBrick(glm::ivec3 pos, const Brick& brick) {
    getBrick(pos) = brick;
}

void vxe::BrickMap::setVoxel(glm::ivec3 pos, Material material) {
    glm::ivec3 brickPos = pos / BRICK_SIZE;
    glm::ivec3 relVoxelPos = pos % BRICK_SIZE;
    getBrick(brickPos).voxels[relVoxelPos.x][relVoxelPos.y][relVoxelPos.z].material = material;
}

void vxe::BrickMap::fillRegion(glm::ivec3 position, glm::ivec3 extents, Material material) {
    for (int x = position.x; x < position.x + extents.x; x++) {
        for (int y = position.y; y < position.y + extents.y; y++) {
            for (int z = position.z; z < position.z + extents.z; z++) {
                setVoxel(glm::ivec3(x, y, z), material);
            }
        }
    }
}

bool vxe::BrickMap::generateChunk(const glm::ivec3 &pos) {
    std::lock_guard lock(m_chunkGenMutex);

    if(pos.x >= dimensions.x || pos.y >= dimensions.y || pos.z >= dimensions.z ||
       pos.x < 0 || pos.y < 0 || pos.z < 0)
        return false;

    for (int z = 0; z < BRICK_SIZE; z++) {
        for (int x = 0; x < BRICK_SIZE; x++) {
            int yTop = (m_noise.GetNoise((float) (x + pos.x * BRICK_SIZE), (float) (z + pos.z * BRICK_SIZE)) + 1.0) / 2.0 * ((dimensions.y * BRICK_SIZE) / 4.0) + 1.0;

            if (yTop < pos.y * BRICK_SIZE) continue;

            if (yTop < pos.y * BRICK_SIZE + BRICK_SIZE)
                setVoxel(glm::ivec3(x + pos.x * BRICK_SIZE, yTop, z + pos.z * BRICK_SIZE), Material::GRASS);

            int localYTop = std::min(yTop - pos.y * BRICK_SIZE, BRICK_SIZE);

            for (int y = 0; y < localYTop; y++) {
                setVoxel(glm::ivec3(x + pos.x * BRICK_SIZE, y + pos.y * BRICK_SIZE, z + pos.z * BRICK_SIZE), Material::STONE);
            }
        }
    }

    return true;
}

size_t vxe::BrickMap::getSize() {
    return bricks.size();
}

std::vector<vxe::Brick>& vxe::BrickMap::getBricks() {
    return bricks;
}

vxe::GPUGrid vxe::BrickMap::getGPUGrid() {
    GPUGrid ret{};

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
                    if (brick.voxels[x][y][z].material != Material::AIR) {
                        // CORRECT voxel index calculation (matches your shader!)
                        uint32_t voxelIndex = x + y * BRICK_SIZE + z * BRICK_SIZE * BRICK_SIZE;
                        uint32_t wordIndex = voxelIndex / 64;
                        uint32_t bitIndex = voxelIndex % 64;
                        
                        gpuBrick.bitmask[wordIndex] |= 1ULL << bitIndex;
                        ret.materials.push_back(static_cast<uint32_t>(brick.voxels[x][y][z].material));
                    }
                }
            }
        }
        ret.bricks.push_back(gpuBrick);
    }

    return ret;
}

size_t vxe::BrickMap::getSizeInBytes() {
    int brickSize = bricks.size() * sizeof(Brick);
    int indexSize = indexTable.size() * (sizeof(glm::ivec3) + sizeof(int));
    return brickSize + indexSize + sizeof(BrickMap);
}