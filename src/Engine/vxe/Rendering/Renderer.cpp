#include "Renderer.h"

namespace vxe {
    void Renderer::init() {
        m_api = RenderAPI::create();
        m_api->init();
    }

    void Renderer::beginFrame() {
        m_api->clear();
    }

    void Renderer::submit(Renderable* object) {
        m_renderQueue.push_back(object);
    }

    void Renderer::endFrame() {
        for (const auto& obj : m_renderQueue) {
            obj->draw(m_api);
        }
        m_renderQueue.clear();
    }

    RenderAPI* Renderer::getAPI() {
        return m_api;
    }
}