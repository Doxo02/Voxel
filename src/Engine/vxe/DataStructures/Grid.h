#ifndef GRID_H
#define GRID_H

#include <cstddef>
#include <memory>
#include <glm/glm.hpp>

namespace vxe {
    enum class Material {
        AIR = 0,
        GRASS = 1,
        STONE = 2
    };

    struct alignas(16) MaterialInfo {
        glm::vec4 albedo;
        float metallic;
        float roughness;
        float padding = 0.0f;
    };

    enum class GridType {
        BRICK_MAP
    };

    struct GPUGrid;

    class Grid {
        public:
            virtual ~Grid() = default;

            virtual void setVoxel(glm::ivec3 position, Material material) = 0; // TODO: find modular solution for material
            virtual void fillRegion(glm::ivec3 position, glm::ivec3 extents, Material material) = 0;
            virtual bool generateChunk(const glm::ivec3& pos) = 0;

            virtual GPUGrid getGPUGrid() = 0;
            virtual size_t getSize() = 0;
            virtual size_t getSizeInBytes() = 0;

            static std::unique_ptr<Grid> create(const GridType& type, const glm::ivec3& dimensions);
    };
}

#endif