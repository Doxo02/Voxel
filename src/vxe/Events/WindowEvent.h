#ifndef VXE_WINDOW_EVENT_H
#define VXE_WINDOW_EVENT_H

#include "Event.h"

namespace vxe {

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_width(width), m_height(height) {}

        unsigned int getWidth() const { return m_width; }
        unsigned int getHeight() const { return m_height; }

        std::string toString() const override {
            return "WindowResizeEvent: " + std::to_string(m_width) + ", " + std::to_string(m_height);
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryWindow)

    private:
        unsigned int m_width, m_height;
    };

    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class WindowFocusEvent : public Event {
    public:
        WindowFocusEvent() = default;

        EVENT_CLASS_TYPE(WindowFocus)
        EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class WindowLostFocusEvent : public Event {
    public:
        WindowLostFocusEvent() = default;

        EVENT_CLASS_TYPE(WindowLostFocus)
        EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class WindowMovedEvent : public Event {
    public:
        WindowMovedEvent(int x, int y) : m_x(x), m_y(y) {}

        int getX() const { return m_x; }
        int getY() const { return m_y; }

        std::string toString() const override {
            return "WindowMovedEvent: " + std::to_string(m_x) + ", " + std::to_string(m_y);
        }

        EVENT_CLASS_TYPE(WindowMoved)
        EVENT_CLASS_CATEGORY(EventCategoryWindow)

    private:
        int m_x, m_y;
    };

} // namespace vxe

#endif // VXE_WINDOW_EVENT_H
