#ifndef VXE_MOUSE_EVENT_H
#define VXE_MOUSE_EVENT_H

#include "Event.h"

namespace vxe {

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) : m_mouseX(x), m_mouseY(y) {}

        float getX() const { return m_mouseX; }
        float getY() const { return m_mouseY; }

        std::string toString() const override {
            return "MouseMovedEvent: " + std::to_string(m_mouseX) + ", " + std::to_string(m_mouseY);
        }

        EVENT_CLASS_TYPE(MouseMoved)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float m_mouseX, m_mouseY;
    };

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float xOffset, float yOffset)
            : m_xOffset(xOffset), m_yOffset(yOffset) {}

        float getXOffset() const { return m_xOffset; }
        float getYOffset() const { return m_yOffset; }

        std::string toString() const override {
            return "MouseScrolledEvent: " + std::to_string(m_xOffset) + ", " + std::to_string(m_yOffset);
        }

        EVENT_CLASS_TYPE(MouseScrolled)
        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        float m_xOffset, m_yOffset;
    };

    class MouseButtonEvent : public Event {
    public:
        int getMouseButton() const { return m_button; }

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

    protected:
        MouseButtonEvent(int button) : m_button(button) {}

        int m_button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

        std::string toString() const override {
            return "MouseButtonPressedEvent: " + std::to_string(m_button);
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

        std::string toString() const override {
            return "MouseButtonReleasedEvent: " + std::to_string(m_button);
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

} // namespace vxe

#endif // VXE_MOUSE_EVENT_H
