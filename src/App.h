#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "Rendering/Camera.h"

#include <vxe.h>

/// @brief The main App which contains most of the logic.
class App : public vxe::Application {
    public:
        App(int width, int height, const char* title);
        bool init();
        void terminate();

        void run() override;
    
    private:
        int m_width, m_height;
        const char* m_title;

        std::unique_ptr<vxe::Window> m_window;

        std::unique_ptr<Camera> m_camera;
        glm::mat4x4 m_projection;

        std::unique_ptr<vxe::Renderer> m_renderer;

        std::unique_ptr<vxe::Shader> m_program;

        std::unique_ptr<vxe::ShaderStorageBuffer> m_brickMapSSBO;
        std::unique_ptr<vxe::ShaderStorageBuffer> m_brickSSBO;
        std::unique_ptr<vxe::ShaderStorageBuffer> m_materialSSBO;
        std::unique_ptr<vxe::ShaderStorageBuffer> m_materialInfosSSBO;
        std::unique_ptr<vxe::VoxelGrid> m_grid;

        bool viewportResized = false;
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        bool pressingESC = false;
        double lastX = 400;
        double lastY = 400;
        bool cursorEnabled = false;

        bool running = true;

        void processInput();
        bool onResize(vxe::WindowResizeEvent& e);
        bool onKeyPressed(vxe::KeyPressedEvent& e);
        bool onKeyReleased(vxe::KeyReleasedEvent& e);
        bool onMouseMove(vxe::MouseMovedEvent& e);
        bool onMouseScroll(vxe::MouseScrolledEvent& e);
        bool onWindowClose(vxe::WindowCloseEvent& e);
};