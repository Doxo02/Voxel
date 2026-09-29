#ifndef VXE_KEY_EVENT_H
#define VXE_KEY_EVENT_H

#include "Event.h"

namespace vxe {

    class KeyEvent : public Event {
    public:
        int getKeyCode() const { return m_keyCode; }

        EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

    protected:
        KeyEvent(int keycode) : m_keyCode(keycode) {}

        int m_keyCode;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(int keycode, int repeatCount)
            : KeyEvent(keycode), m_repeatCount(repeatCount) {}

        int getRepeatCount() const { return m_repeatCount; }

        std::string toString() const override {
            return "KeyPressedEvent: " + std::to_string(m_keyCode) + " (" + std::to_string(m_repeatCount) + " repeats)";
        }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        int m_repeatCount;
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

        std::string toString() const override {
            return "KeyReleasedEvent: " + std::to_string(m_keyCode);
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };

    class KeyTypedEvent : public KeyEvent {
    public:
        KeyTypedEvent(int keycode) : KeyEvent(keycode) {}

        std::string toString() const override {
            return "KeyTypedEvent: " + std::to_string(m_keyCode);
        }

        EVENT_CLASS_TYPE(KeyTyped)
    };

} // namespace vxe

#endif // VXE_KEY_EVENT_H
