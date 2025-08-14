#include "ogl_RenderAPI.h"

#include <GL/glew.h>
#include <spdlog/spdlog.h>

void vxe::OGLRenderAPI::init() {
    if(glewInit() != GLEW_OK){
        // spdlog::critical("Failed to initialize GLEW!");
        throw std::runtime_error("Failed to initialize GLEW!");
    }
    spdlog::info("Initialized GLEW.");

    glEnable(GL_DEPTH_TEST);
}

void vxe::OGLRenderAPI::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void vxe::OGLRenderAPI::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void vxe::OGLRenderAPI::drawVertexArray(const vxe::VertexArray* va, unsigned int count) {
    va->bind();
    glDrawArrays(GL_TRIANGLES, 0, count); // TODO: switch to draw elements
}

void vxe::OGLRenderAPI::drawElements(const vxe::VertexArray* va, unsigned int count) {
    va->bind();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
}

void vxe::OGLRenderAPI::setClearColor(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
}