#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Rendering/Camera.h"

#include "DataStructures/BrickMap.h"
#include "DataStructures/World.h"

#include "GL_Abstract/Program.h"
#include "GL_Abstract/ShaderStorageBuffer.h"
#include "GL_Abstract/VertexArray.h"

/// @brief The main App which contains most of the logic.
class App {
    public:
        App(int width, int height, const char* title);
        bool init();
        void terminate();

        void run();
    
    private:
        int m_width, m_height;
        const char* m_title;

        GLFWwindow* m_window;

        Camera* m_camera;
        glm::mat4x4 m_projection;

        gla::Program* m_program;

        gla::ShaderStorageBuffer* m_brickMapSSBO;
        gla::ShaderStorageBuffer* m_brickSSBO;
        gla::ShaderStorageBuffer* m_materialSSBO;
        gla::ShaderStorageBuffer* m_materialInfosSSBO;
        gla::VertexArray* m_dummyVAO;

        World* m_world;

        bool viewportResized = false;
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        bool pressingESC = false;
        double lastX = 400;
        double lastY = 400;
        bool cursorEnabled = false;

        static void onResize(GLFWwindow* window, int width, int height);
        static void processInput(GLFWwindow* window);
        static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};