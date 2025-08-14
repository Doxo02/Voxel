//
// Created by lars on 13.08.25.
//

#include "Grid.h"

#include <memory>
#include "BrickMap.h"

namespace vxe
{
    std::unique_ptr<Grid> Grid::create(const GridType& type, const glm::ivec3& dimensions) {
        switch (type) {
        case GridType::BRICK_MAP: {
                return std::make_unique<BrickMap>(dimensions);
        } break;
        }

        return nullptr;
    }
}