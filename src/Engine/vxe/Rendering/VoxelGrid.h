#ifndef VOXELGRID_H
#define VOXELGRID_H

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
            VertexArray* m_vao;
            Grid* m_grid;
    };
}

#endif