//
// Created by silas on 29.07.26.
//

#ifndef LUMADI_EVENTS_EVENT_BUS_H
#define LUMADI_EVENTS_EVENT_BUS_H

#include <unordered_map>
#include <vector>
#include <typeindex>
#include <functional>

namespace Lumadi::Events {
    class EventBus {
    public:
        template<typename T>
        using Callback = std::function<void(const T &)>;

        template<typename T>
        void Subscribe(Callback<T> callback) {
            auto wrapper = [callback](const void *event) {
                callback(*static_cast<const T *>(event));
            };

            mSubscribers[typeid(T)].push_back(std::move(wrapper));
        }

        template<typename T>
        void Public(const T &event) {
            const auto it = mSubscribers.find(typeid(T));


            if (it == mSubscribers.end()) {
                return;
            }

            for (auto &callback: it->second) {
                callback(&event);
            }
        }

    private:
        std::unordered_map<std::type_index, std::vector<std::function<void(const void *)> > > mSubscribers;
    };
}

#endif //LUMADI_EVENTS_EVENT_BUS_H
