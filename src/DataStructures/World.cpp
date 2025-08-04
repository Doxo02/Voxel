#include "World.h"

#include <mutex>

World::World(glm::ivec3 worldSize, int seed) : m_worldSize(worldSize), m_map(worldSize), m_seed(seed) {
    m_noise = FastNoiseLite(m_seed);
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

World::~World() {}

bool World::generateBrick(const glm::ivec3 &pos) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    if(pos.x >= m_worldSize.x || pos.y >= m_worldSize.y || pos.z >= m_worldSize.z ||
       pos.x < 0 || pos.y < 0 || pos.z < 0) 
        return false;

    for (int z = 0; z < BRICK_SIZE; z++) {
        for (int x = 0; x < BRICK_SIZE; x++) {
            int yTop = (m_noise.GetNoise((float) (x + pos.x * BRICK_SIZE), (float) (z + pos.z * BRICK_SIZE)) + 1.0) / 2.0 * ((m_worldSize.y * BRICK_SIZE) / 4.0) + 1.0;

            if (yTop < pos.y * BRICK_SIZE) continue;

            if (yTop < pos.y * BRICK_SIZE + BRICK_SIZE)
                m_map.setVoxel(glm::ivec3(x + pos.x * BRICK_SIZE, yTop, z + pos.z * BRICK_SIZE), GRASS);
            
            int localYTop = std::min(yTop - pos.y * BRICK_SIZE, BRICK_SIZE);

            for (int y = 0; y < localYTop; y++) {
                m_map.setVoxel(glm::ivec3(x + pos.x * BRICK_SIZE, y + pos.y * BRICK_SIZE, z + pos.z * BRICK_SIZE), STONE);
            }
        }
    }
    
    return true;
}

BrickMap& World::getMap() {
    return m_map;
}