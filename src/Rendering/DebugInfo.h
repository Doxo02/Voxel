#ifndef DEBUGINFO_H
#define DEBUGINFO_H

#include <vxe/Rendering/Renderable.h>

class DebugInfo : public vxe::Renderable {
    public:
        ~DebugInfo() override;

        void init(vxe::Window* window);
        void updateValues(glm::vec3 cameraPos, float memory);
        float getVoxelScale();
        glm::vec3 getLightPos();
        glm::vec3 getLightColor();
        float getLightIntensity();
        void draw(vxe::RenderAPI* api) override;
    
    private:
        float m_memory, m_lightIntensity, m_voxelScale;
        glm::vec3 m_cameraPos, m_lightPos, m_lightColor;
};

#endif