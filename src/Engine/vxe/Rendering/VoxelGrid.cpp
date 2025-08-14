#include "VoxelGrid.h"


vxe::VoxelGrid::VoxelGrid(const GridType type, const glm::ivec3 dimensions) {
    m_grid = Grid::create(type, dimensions);
    m_vao = VertexArray::create();
}

vxe::Grid* vxe::VoxelGrid::getGrid() const
{
    return m_grid;
}

void vxe::VoxelGrid::bindVA() {
    m_vao->bind();
}

void vxe::VoxelGrid::draw(RenderAPI* api) {
    api->drawVertexArray(m_vao, 3);
}