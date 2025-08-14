#include "Renderer.h"

namespace vxe {
    void Renderer::init(Window* window) {
        m_api = RenderAPI::create();
        m_api->init(window);
        m_window = window;
    }

    void Renderer::beginFrame() {
        m_api->clear();
    }

    void Renderer::submit(Renderable* object) {
        m_renderQueue.push_back(object);
    }

    void Renderer::endFrame() {
        for (const auto& obj : m_renderQueue) {
            obj->draw(m_api.get());
        }
        m_renderQueue.clear();
        m_api->swapBuffer(m_window);
    }

    RenderAPI* Renderer::getAPI() {
        return m_api.get();
    }
}