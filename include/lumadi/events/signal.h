//
// Created by silas on 29.07.26.
//

#ifndef LUMADI_EVENTS_SIGNAL_H
#define LUMADI_EVENTS_SIGNAL_H
#include <functional>

namespace Lumadi::Events {
    template<typename... Args>
    class Signal {
    public:
        using Slot = std::function<void(Args...)>;

        void Connect(Slot slot) {
            slots.push_back(std::move(slot));
        }

        template<typename... CallArgs>
        void Emit(CallArgs &&... args) {
            for (auto &slot: slots) {
                slot(std::forward<CallArgs>(args)...);
            }
        }

    private:
        std::vector<Slot> slots;
    };
}

#endif //LUMADI_EVENTS_SIGNAL_H
