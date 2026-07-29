//
// Created by silas on 29.07.26.
//

#include "catch2/catch_test_macros.hpp"
#include "catch2/catch_approx.hpp"
#include "lumadi/events/signal.h"
#include <string>
#include <vector>
#include <memory>

using namespace Lumadi::Events;

TEST_CASE("Signal with single slot", "[signal]") {
    Signal<int> signal;
    int received = 0;

    signal.Connect([&](int value) {
        received = value;
    });

    signal.Emit(42);

    REQUIRE(received == 42);
}

TEST_CASE("Signal with multiple slots", "[signal]") {
    Signal<int> signal;
    int count = 0;

    signal.Connect([&](int) { ++count; });
    signal.Connect([&](int) { ++count; });
    signal.Connect([&](int) { ++count; });

    signal.Emit(1);

    REQUIRE(count == 3);
}

TEST_CASE("Signal with no slots emits without error", "[signal]") {
    Signal<int> signal;

    REQUIRE_NOTHROW(signal.Emit(42));
}

TEST_CASE("Signal passes arguments correctly", "[signal]") {
    Signal<int> signal;
    int received = 0;

    signal.Connect([&](int value) {
        received = value;
    });

    signal.Emit(99);
    REQUIRE(received == 99);

    signal.Emit(-1);
    REQUIRE(received == -1);
}

TEST_CASE("Signal with multiple argument types", "[signal]") {
    Signal<int, double, std::string> signal;
    int ri = 0;
    double rd = 0.0;
    std::string rs;

    signal.Connect([&](int i, double d, const std::string &s) {
        ri = i;
        rd = d;
        rs = s;
    });

    signal.Emit(7, 3.14, "hello");

    REQUIRE(ri == 7);
    REQUIRE(rd == Catch::Approx(3.14));
    REQUIRE(rs == "hello");
}

TEST_CASE("Signal with lambda capture by reference", "[signal]") {
    Signal<> signal;
    int counter = 0;

    signal.Connect([&] { ++counter; });
    signal.Emit();

    REQUIRE(counter == 1);
}

TEST_CASE("Signal with function pointer", "[signal]") {
    static int value = 0;
    auto reset = [] { value = 0; };
    reset();

    auto func = [](int x) { value = x; };

    Signal<int> signal;
    signal.Connect(func);

    signal.Emit(7);
    REQUIRE(value == 7);
}

TEST_CASE("Signal with std::function slot", "[signal]") {
    Signal<int> signal;
    int result = 0;

    std::function<void(int)> slot = [&](int x) { result = x * 2; };
    signal.Connect(slot);

    signal.Emit(5);
    REQUIRE(result == 10);
}

TEST_CASE("Signal multiple emits", "[signal]") {
    Signal<int> signal;
    int sum = 0;

    signal.Connect([&](int value) { sum += value; });

    signal.Emit(10);
    signal.Emit(20);
    signal.Emit(30);

    REQUIRE(sum == 60);
}

TEST_CASE("Signal preserves slot invocation order", "[signal]") {
    Signal<int> signal;
    std::vector<int> order;

    signal.Connect([&](int) { order.push_back(1); });
    signal.Connect([&](int) { order.push_back(2); });
    signal.Connect([&](int) { order.push_back(3); });

    signal.Emit(0);

    REQUIRE(order.size() == 3);
    REQUIRE(order[0] == 1);
    REQUIRE(order[1] == 2);
    REQUIRE(order[2] == 3);
}

TEST_CASE("Signal with move-only argument", "[signal]") {
    Signal<std::unique_ptr<int>> signal;
    bool called = false;

    signal.Connect([&](std::unique_ptr<int> ptr) {
        called = true;
        REQUIRE(*ptr == 42);
    });

    auto p = std::make_unique<int>(42);
    signal.Emit(std::move(p));

    REQUIRE(called);
}

TEST_CASE("Signal with const reference argument", "[signal]") {
    Signal<const std::string &> signal;
    const std::string *addr = nullptr;

    signal.Connect([&](const std::string &s) {
        addr = &s;
    });

    std::string data = "hello";
    signal.Emit(data);

    REQUIRE(addr == &data);
}

TEST_CASE("Signal with mutable lambda", "[signal]") {
    Signal<int> signal;
    int count = 0;

    signal.Connect([count](int) mutable { ++count; });

    signal.Emit(1);
    signal.Emit(1);

    SUCCEED("Mutable lambda connected and emitted without issue");
}

TEST_CASE("Signal with void signature (no args)", "[signal]") {
    Signal<> signal;
    int called = 0;

    signal.Connect([&] { ++called; });
    signal.Emit();

    REQUIRE(called == 1);
}

TEST_CASE("Signal copy semantics", "[signal]") {
    Signal<int> original;
    int count = 0;

    original.Connect([&](int) { ++count; });

    Signal<int> copy = original;
    copy.Emit(1);

    REQUIRE(count == 1);
}

TEST_CASE("Signal move semantics", "[signal]") {
    Signal<int> original;
    int count = 0;

    original.Connect([&](int) { ++count; });

    Signal<int> moved = std::move(original);
    moved.Emit(1);

    REQUIRE(count == 1);
}

TEST_CASE("Signal connect after emitting", "[signal]") {
    Signal<int> signal;
    int first = 0;
    int second = 0;

    signal.Connect([&](int) { first = 1; });
    signal.Emit(0);
    REQUIRE(first == 1);
    REQUIRE(second == 0);

    signal.Connect([&](int) { second = 2; });
    signal.Emit(0);
    REQUIRE(first == 1);
    REQUIRE(second == 2);
}

TEST_CASE("Signal with multiple connected lambdas modifying different captures", "[signal]") {
    Signal<int> signal;
    int a = 0, b = 0, c = 0;

    signal.Connect([&](int v) { a += v; });
    signal.Connect([&](int v) { b += v; });
    signal.Connect([&](int v) { c += v; });

    signal.Emit(1);
    signal.Emit(2);

    REQUIRE(a == 3);
    REQUIRE(b == 3);
    REQUIRE(c == 3);
}

TEST_CASE("Signal with member function pointer via bind", "[signal]") {
    struct Receiver {
        int value = 0;
        void set(int v) { value = v; }
    };

    Receiver rec;
    Signal<int> signal;

    signal.Connect(std::bind(&Receiver::set, &rec, std::placeholders::_1));
    signal.Emit(99);

    REQUIRE(rec.value == 99);
}

TEST_CASE("Signal with large number of slots", "[signal]") {
    constexpr int N = 100;
    Signal<int> signal;
    int count = 0;

    for (int i = 0; i < N; ++i) {
        signal.Connect([&](int) { ++count; });
    }

    signal.Emit(0);
    REQUIRE(count == N);
}
