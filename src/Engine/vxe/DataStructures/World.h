#ifndef WORLD_H
#define WORLD_H

#include <glm/glm.hpp>
#include <FastNoiseLite.h>

#include "BrickMap.h"

namespace vxe {
    class World {
    public:
        World(glm::ivec3 worldSize, int seed = 0);
        ~World();

        bool generateBrick(const glm::ivec3 &pos);

        BrickMap* getMap();

    private:
        int m_seed = 0;
        FastNoiseLite m_noise;

        glm::ivec3 m_worldSize;
        BrickMap* m_map;
    };
}

#endif // WORLD_H