#ifndef VXE_APPLICATION_EVENT_H
#define VXE_APPLICATION_EVENT_H

#include "Event.h"

namespace vxe {

    class ApplicationTickEvent : public Event {
    public:
        ApplicationTickEvent() = default;

        EVENT_CLASS_TYPE(ApplicationTick)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

    class ApplicationUpdateEvent : public Event {
    public:
        ApplicationUpdateEvent(float deltaTime) : m_deltaTime(deltaTime) {}

        float getDeltaTime() const { return m_deltaTime; }

        std::string toString() const override {
            return "ApplicationUpdateEvent: " + std::to_string(m_deltaTime) + "s";
        }

        EVENT_CLASS_TYPE(ApplicationUpdate)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

    private:
        float m_deltaTime;
    };

    class ApplicationRenderEvent : public Event {
    public:
        ApplicationRenderEvent() = default;

        EVENT_CLASS_TYPE(ApplicationRender)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };

} // namespace vxe

#endif // VXE_APPLICATION_EVENT_H
