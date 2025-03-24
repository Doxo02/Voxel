#ifndef BRICKMAP_H
#define BRICKMAP_H

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <array>

struct BrickMap {
    uint64_t solidMask[8] = {
        0x0000000000000000,
        0x0000000000000001,
        0x0000000000000003,
        0x0000000000000007,
        0x000000000000000F,
        0x000000000000001F,
        0x000000000000003F,
        0x000000000000007F
    };
    
    uint32_t colorOffset;
    glm::ivec3 position;
};

class BrickGrid {
    std::vector<BrickMap> bricks;

public:
    BrickGrid(int size) {
        bricks.resize(size * size * size);
    }

    BrickMap& getBrick(int x, int y, int z) {
        return bricks[x + y * 16 + z * 256];
    }

    void setBrick(int x, int y, int z, const BrickMap& brick) {
        bricks[x + y * 16 + z * 256] = brick;
    }

    int getSize() {
        return bricks.size();
    }

    std::vector<BrickMap>& getBricks() {
        return bricks;
    }
};

#endif // BRICKMAP_H