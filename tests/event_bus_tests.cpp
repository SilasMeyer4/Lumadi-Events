//
// Created by silas on 29.07.26.
//

#include "catch2/catch_test_macros.hpp"
#include "lumadi/events/event_bus.h"
#include <string>
#include <vector>

using namespace Lumadi::Events;

TEST_CASE("EventBus subscribe and publish", "[event_bus]") {
    EventBus bus;
    int received = 0;

    bus.Subscribe<int>([&](const int &value) {
        received = value;
    });

    bus.Public(42);

    REQUIRE(received == 42);
}

TEST_CASE("EventBus multiple subscribers to same type", "[event_bus]") {
    EventBus bus;
    int count = 0;

    bus.Subscribe<int>([&](const int &) { ++count; });
    bus.Subscribe<int>([&](const int &) { ++count; });
    bus.Subscribe<int>([&](const int &) { ++count; });

    bus.Public(1);

    REQUIRE(count == 3);
}

TEST_CASE("EventBus no subscribers does not crash", "[event_bus]") {
    EventBus bus;

    REQUIRE_NOTHROW(bus.Public(42));
    REQUIRE_NOTHROW(bus.Public(std::string("hello")));
    REQUIRE_NOTHROW(bus.Public(3.14));
}

TEST_CASE("EventBus different event types", "[event_bus]") {
    EventBus bus;
    int intVal = 0;
    std::string strVal;

    bus.Subscribe<int>([&](const int &v) { intVal = v; });
    bus.Subscribe<std::string>([&](const std::string &v) { strVal = v; });

    bus.Public(100);
    REQUIRE(intVal == 100);
    REQUIRE(strVal == "");

    bus.Public(std::string("world"));
    REQUIRE(intVal == 100);
    REQUIRE(strVal == "world");
}

TEST_CASE("EventBus selective dispatch", "[event_bus]") {
    EventBus bus;
    int intCount = 0;
    int strCount = 0;

    bus.Subscribe<int>([&](const int &) { ++intCount; });
    bus.Subscribe<std::string>([&](const std::string &) { ++strCount; });

    bus.Public(1);
    bus.Public(2);

    REQUIRE(intCount == 2);
    REQUIRE(strCount == 0);
}

TEST_CASE("EventBus multiple publishes", "[event_bus]") {
    EventBus bus;
    int sum = 0;

    bus.Subscribe<int>([&](const int &v) { sum += v; });

    bus.Public(10);
    bus.Public(20);
    bus.Public(30);

    REQUIRE(sum == 60);
}

TEST_CASE("EventBus with struct event type", "[event_bus]") {
    struct LoginEvent {
        int userId;
        std::string username;
    };

    EventBus bus;
    LoginEvent received{0, ""};

    bus.Subscribe<LoginEvent>([&](const LoginEvent &e) {
        received = e;
    });

    LoginEvent event{42, "alice"};
    bus.Public(event);

    REQUIRE(received.userId == 42);
    REQUIRE(received.username == "alice");
}

TEST_CASE("EventBus capturing subscriber modifies external state", "[event_bus]") {
    EventBus bus;
    std::vector<int> history;

    bus.Subscribe<int>([&](const int &v) {
        history.push_back(v);
    });

    bus.Public(1);
    bus.Public(2);
    bus.Public(3);

    REQUIRE(history.size() == 3);
    REQUIRE(history[0] == 1);
    REQUIRE(history[1] == 2);
    REQUIRE(history[2] == 3);
}

TEST_CASE("EventBus subscribe with const T matches T", "[event_bus]") {
    EventBus bus;
    int received = 0;

    bus.Subscribe<const int>([&](const int &v) {
        received = v;
    });

    bus.Public(99);

    REQUIRE(received == 99);
}



TEST_CASE("EventBus multiple event types with multiple subscribers each", "[event_bus]") {
    struct EventA { int x; };
    struct EventB { double y; };

    EventBus bus;
    int aCount = 0;
    int bCount = 0;

    bus.Subscribe<EventA>([&](const EventA &) { ++aCount; });
    bus.Subscribe<EventA>([&](const EventA &) { ++aCount; });
    bus.Subscribe<EventB>([&](const EventB &) { ++bCount; });
    bus.Subscribe<EventB>([&](const EventB &) { ++bCount; });
    bus.Subscribe<EventB>([&](const EventB &) { ++bCount; });

    bus.Public(EventA{1});
    REQUIRE(aCount == 2);
    REQUIRE(bCount == 0);

    bus.Public(EventB{3.14});
    REQUIRE(aCount == 2);
    REQUIRE(bCount == 3);
}

TEST_CASE("EventBus with many subscribers", "[event_bus]") {
    constexpr int N = 100;
    EventBus bus;
    int count = 0;

    for (int i = 0; i < N; ++i) {
        bus.Subscribe<int>([&](const int &) { ++count; });
    }

    bus.Public(0);

    REQUIRE(count == N);
}

TEST_CASE("EventBus with complex event struct containing containers", "[event_bus]") {
    struct BatchEvent {
        std::vector<int> ids;
        std::string label;
    };

    EventBus bus;
    BatchEvent received{};

    bus.Subscribe<BatchEvent>([&](const BatchEvent &e) {
        received = e;
    });

    BatchEvent event{{1, 2, 3}, "test"};
    bus.Public(event);

    REQUIRE(received.label == "test");
    REQUIRE(received.ids.size() == 3);
    REQUIRE(received.ids[0] == 1);
    REQUIRE(received.ids[1] == 2);
    REQUIRE(received.ids[2] == 3);
}

TEST_CASE("EventBus publish with const event", "[event_bus]") {
    EventBus bus;
    int received = 0;

    bus.Subscribe<int>([&](const int &v) {
        received = v;
    });

    const int value = 55;
    bus.Public(value);

    REQUIRE(received == 55);
}

TEST_CASE("EventBus no cross-contamination between event types", "[event_bus]") {
    EventBus bus;
    bool intCalled = false;
    bool stringCalled = false;

    bus.Subscribe<int>([&](const int &) { intCalled = true; });
    bus.Subscribe<std::string>([&](const std::string &) { stringCalled = true; });

    bus.Public(42);

    REQUIRE(intCalled);
    REQUIRE_FALSE(stringCalled);

    intCalled = false;
    bus.Public(std::string("hello"));

    REQUIRE(stringCalled);
    REQUIRE_FALSE(intCalled);
}

TEST_CASE("EventBus move-only event type", "[event_bus]") {
    struct MoveOnlyEvent {
        std::unique_ptr<int> data;
        explicit MoveOnlyEvent(int v) : data(std::make_unique<int>(v)) {}
    };

    EventBus bus;
    int received = 0;

    bus.Subscribe<MoveOnlyEvent>([&](const MoveOnlyEvent &e) {
        received = *e.data;
    });

    MoveOnlyEvent event(42);
    bus.Public(event);

    REQUIRE(received == 42);
}
