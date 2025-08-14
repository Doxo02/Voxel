#ifndef VXE_EVENT_MANAGER_H
#define VXE_EVENT_MANAGER_H

#include "Event.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include <typeindex>

namespace vxe {

    // Type-safe event callback wrapper
    template<typename EventType>
    using EventCallback = std::function<bool(EventType&)>;

    // Base listener interface
    class IEventListener {
    public:
        virtual ~IEventListener() = default;
        virtual bool handleEvent(Event& event) = 0;
        virtual std::type_index getEventType() const = 0;
    };

    // Templated event listener
    template<typename EventType>
    class EventListener : public IEventListener {
    public:
        EventListener(EventCallback<EventType> callback) : m_callback(callback) {}

        bool handleEvent(Event& event) override {
            return m_callback(static_cast<EventType&>(event));
        }

        std::type_index getEventType() const override {
            return std::type_index(typeid(EventType));
        }

    private:
        EventCallback<EventType> m_callback;
    };

    class EventManager {
    public:
        static EventManager& getInstance() {
            static EventManager instance;
            return instance;
        }

        // Subscribe to an event type with a callback
        template<typename EventType>
        void subscribe(EventCallback<EventType> callback) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto listener = std::make_shared<EventListener<EventType>>(callback);
            m_listeners[std::type_index(typeid(EventType))].push_back(listener);
        }

        // Subscribe to an event type with a member function
        template<typename EventType, typename T>
        void subscribe(T* instance, bool(T::*memberFunc)(EventType&)) {
            subscribe<EventType>([instance, memberFunc](EventType& event) {
                return (instance->*memberFunc)(event);
            });
        }

        // Unsubscribe from an event type (removes all listeners of that type)
        template<typename EventType>
        void unsubscribe() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_listeners.erase(std::type_index(typeid(EventType)));
        }

        // Dispatch an event immediately
        template<typename EventType, typename... Args>
        void dispatch(Args&&... args) {
            auto event = std::make_unique<EventType>(std::forward<Args>(args)...);
            dispatchEvent(*event);
        }

        // Queue an event for later processing
        template<typename EventType, typename... Args>
        void queueEvent(Args&&... args) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_eventQueue.push(std::make_unique<EventType>(std::forward<Args>(args)...));
        }

        // Process all queued events
        void processEvents() {
            std::lock_guard<std::mutex> lock(m_mutex);
            while (!m_eventQueue.empty()) {
                auto& event = m_eventQueue.front();
                dispatchEvent(*event);
                m_eventQueue.pop();
            }
        }

        // Direct event dispatch
        void dispatchEvent(Event& event) {
            std::type_index eventType = std::type_index(typeid(event));
            
            auto it = m_listeners.find(eventType);
            if (it != m_listeners.end()) {
                for (auto& listener : it->second) {
                    if (event.isHandled()) break;
                    listener->handleEvent(event);
                }
            }
        }

        // Clear all listeners
        void clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_listeners.clear();
            std::queue<std::unique_ptr<Event>> empty;
            m_eventQueue.swap(empty);
        }

        // Get number of listeners for a specific event type
        template<typename EventType>
        size_t getListenerCount() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_listeners.find(std::type_index(typeid(EventType)));
            return it != m_listeners.end() ? it->second.size() : 0;
        }

        // Check if there are any events in the queue
        bool hasQueuedEvents() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return !m_eventQueue.empty();
        }

        size_t getQueuedEventCount() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_eventQueue.size();
        }

    private:
        EventManager() = default;
        ~EventManager() = default;
        EventManager(const EventManager&) = delete;
        EventManager& operator=(const EventManager&) = delete;

        mutable std::mutex m_mutex;
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<IEventListener>>> m_listeners;
        std::queue<std::unique_ptr<Event>> m_eventQueue;
    };

    // Convenience macros
    #define VXE_EVENT_MANAGER vxe::EventManager::getInstance()
    #define VXE_SUBSCRIBE(EventType, callback) VXE_EVENT_MANAGER.subscribe<EventType>(callback)
    #define VXE_SUBSCRIBE_MEMBER(EventType, instance, memberFunc) VXE_EVENT_MANAGER.subscribe<EventType>(instance, memberFunc)
    #define VXE_DISPATCH(EventType, ...) VXE_EVENT_MANAGER.dispatch<EventType>(__VA_ARGS__)
    #define VXE_QUEUE_EVENT(EventType, ...) VXE_EVENT_MANAGER.queueEvent<EventType>(__VA_ARGS__)
    #define VXE_PROCESS_EVENTS() VXE_EVENT_MANAGER.processEvents()

} // namespace vxe

#endif // VXE_EVENT_MANAGER_H
