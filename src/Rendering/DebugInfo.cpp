#include "DebugInfo.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

DebugInfo::~DebugInfo()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugInfo::init(vxe::Window *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window->getNativeWindow()), true);
    ImGui_ImplOpenGL3_Init();

    m_voxelScale = 1.0f;
    m_lightPos = glm::vec3(80.0f, 70.0f, 80.0f);
    m_lightColor = glm::vec3(1.0f);
    m_lightIntensity = 1.0f;
}

void DebugInfo::updateValues(glm::vec3 cameraPos, float memory)
{
    m_cameraPos = cameraPos;
    m_memory = memory;
}

float DebugInfo::getVoxelScale()
{
    return m_voxelScale;
}

glm::vec3 DebugInfo::getLightPos()
{
    return m_lightPos;
}

glm::vec3 DebugInfo::getLightColor()
{
    return m_lightColor;
}

float DebugInfo::getLightIntensity()
{
    return m_lightIntensity;
}

void DebugInfo::draw(vxe::RenderAPI *api)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Info");
    ImGui::Text("%.4f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("Camere pos: (%.2f, %.2f, %.2f)", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
    ImGui::Text("Memory (MiB): %.2f", (float) m_memory / (1024.0 * 1024.0));
    ImGui::SliderFloat("Voxel Scale", &m_voxelScale, 0.0, 2.0);
    ImGui::InputFloat3("Light Pos", glm::value_ptr(m_lightPos));
    ImGui::InputFloat3("Light Color", glm::value_ptr(m_lightColor));
    ImGui::InputFloat("Light Intensity", &m_lightIntensity, 0.01, 0.1);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
