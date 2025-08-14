#ifndef VXE_EVENT_H
#define VXE_EVENT_H

#include <string>
#include <typeindex>

namespace vxe {

    enum class EventType {
        None = 0,
        // Application Events
        ApplicationTick, ApplicationUpdate, ApplicationRender,
        // Window Events
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        // Input Events
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory {
        None = 0,
        EventCategoryApplication    = 1 << 0,
        EventCategoryInput          = 1 << 1,
        EventCategoryKeyboard       = 1 << 2,
        EventCategoryMouse          = 1 << 3,
        EventCategoryMouseButton    = 1 << 4,
        EventCategoryWindow         = 1 << 5
    };

    // Base Event class
    class Event {
    public:
        Event() : m_handled(false) {}
        virtual ~Event() = default;

        virtual EventType getEventType() const = 0;
        virtual const char* getName() const = 0;
        virtual int getCategoryFlags() const = 0;
        virtual std::string toString() const { return getName(); }

        bool isInCategory(EventCategory category) const {
            return getCategoryFlags() & category;
        }

        bool isHandled() const { return m_handled; }
        void setHandled(bool handled = true) { m_handled = handled; }

    protected:
        bool m_handled;
    };

    // Event dispatcher for handling events
    // class EventDispatcher {
    // public:
    //     EventDispatcher(Event& event) : m_event(event) {}
    //
    //     template<typename T, typename F>
    //     bool dispatch(const F& func) {
    //         if (m_event.getEventType() == T::getStaticType()) {
    //             m_event.setHandled(func(static_cast<T&>(m_event)));
    //             return true;
    //         }
    //         return false;
    //     }
    //
    // private:
    //     Event& m_event;
    // };

    // Macros for easy event class definition
    #define EVENT_CLASS_TYPE(type) \
        static EventType getStaticType() { return EventType::type; } \
        virtual EventType getEventType() const override { return getStaticType(); } \
        virtual const char* getName() const override { return #type; }

    #define EVENT_CLASS_CATEGORY(category) \
        virtual int getCategoryFlags() const override { return category; }

} // namespace vxe

#endif // VXE_EVENT_H
