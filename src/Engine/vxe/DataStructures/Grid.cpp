//
// Created by lars on 13.08.25.
//

#include "Grid.h"

#include "BrickMap.h"

namespace vxe
{
    Grid* Grid::create(const GridType& type, const glm::ivec3& dimensions) {
        switch (type) {
        case GridType::BRICK_MAP: {
                return new BrickMap(dimensions);
        } break;
        }

        return nullptr;
    }
}