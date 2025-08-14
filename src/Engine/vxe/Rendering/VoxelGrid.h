#ifndef VOXELGRID_H
#define VOXELGRID_H

#include <memory>
#include "Renderable.h"
#include "../DataStructures/Grid.h"

namespace vxe {
    class VoxelGrid : public Renderable {
        public:
            VoxelGrid(GridType type, glm::ivec3 dimensions);
            ~VoxelGrid() = default;

            void bindVA();
            void draw(RenderAPI* api) override;

            Grid* getGrid() const;

        private:
            std::unique_ptr<VertexArray> m_vao;
            std::unique_ptr<Grid> m_grid;
    };
}

#endif