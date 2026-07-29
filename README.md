# Lumadi-Events

A header-only C++17 library providing **Signal** and **EventBus** primitives for decoupled communication between components. 
Part of my Lumadi library series.


## Requirements

- C++17 or later
- CMake 3.20+ (for FetchContent integration)

## Integration

NOTICE: Aslong as the 1.0 release is not public there could be breaking changes anytime on the master branch.

### CMake FetchContent (recommended)

```cmake
include(FetchContent)

FetchContent_Declare(
    Lumadi_Events
    GIT_REPOSITORY https://github.com/silasmeyer4/Lumadi_Events.git
    GIT_TAG master
)

FetchContent_MakeAvailable(Lumadi_Events)

target_link_libraries(your_target
    PRIVATE
    Lumadi_Events
)
```

### Manual copy

Copy the `include/lumadi/` directory into your project and add the include path. No compilation required.

## Includes

```cpp
// Everything
#include <lumadi/events/events.h>

// Or individually:
#include <lumadi/events/signal.h>
#include <lumadi/events/event_bus.h>
```

---

## Signal

`Signal<Args...>` is a typed, ordered invocation list. Connect slots (callbacks) and emit to call all of them in order.

### Basic usage

```cpp
#include <lumadi/events/signal.h>
#include <iostream>

using namespace Lumadi::Events;

Signal<int> signal;

// Connect a slot
signal.Connect([](int value) {
    std::cout << "Received: " << value << '\n';
});

// Emit the signal
signal.Emit(42); // prints "Received: 42"
```

### Multiple slots

```cpp
Signal<int> signal;
int count = 0;

signal.Connect([&](int) { ++count; });
signal.Connect([&](int) { ++count; });

signal.Emit(1);
// count == 2
```

### Multiple arguments

```cpp
Signal<int, double, std::string> signal;

signal.Connect([](int i, double d, const std::string &s) {
    // i == 7, d == 3.14, s == "hello"
});

signal.Emit(7, 3.14, "hello");
```

### No arguments

```cpp
Signal<> signal;

signal.Connect([] { std::cout << "fired\n"; });
signal.Emit();
```

### Move-only arguments

```cpp
Signal<std::unique_ptr<int>> signal;

signal.Connect([](std::unique_ptr<int> ptr) {
    // ownership transferred
});

signal.Emit(std::make_unique<int>(42));
```

---

## EventBus

`EventBus` routes events to subscribers based on **event type**. Events are arbitrary types (structs, primitives, etc.).

### Basic usage

```cpp
#include <lumadi/events/event_bus.h>
#include <iostream>

using namespace Lumadi::Events;

EventBus bus;

bus.Subscribe<int>([](const int &value) {
    std::cout << "Received integer: " << value << '\n';
});

bus.Publish(42);
```

### Custom event types

```cpp
struct LoginEvent {
    int userId;
    std::string username;
};

EventBus bus;

bus.Subscribe<LoginEvent>([](const LoginEvent &e) {
    std::cout << "User " << e.username << " logged in\n";
});

LoginEvent event{42, "alice"};
bus.Publish(event);
```

### Multiple subscribers, multiple types

```cpp
EventBus bus;
int intCount = 0;
int strCount = 0;

bus.Subscribe<int>([](const int &) { ++intCount; });
bus.Subscribe<std::string>([](const std::string &) { ++strCount; });

bus.Publish(1);    // intCount == 1, strCount == 0
bus.Publish(2);    // intCount == 2, strCount == 0
```

### Publishing with no subscribers is safe

```cpp
EventBus bus;
bus.Publish(42);           // no-op, no crash
bus.Publish(LoginEvent{}); // no-op, no crash
```

---

## API Reference

### `Signal<Args...>`

| Method | Description |
|---|---|
| `Connect(Slot slot)` | Register a callback (`void(Args...)`) |
| `Emit(CallArgs&&... args)` | Invoke all connected slots in order |

### `EventBus`

| Method                               | Description |
|--------------------------------------|---|
| `Subscribe<T>(Callback<T> callback)` | Register a callback for events of type `T` |
| `Publish(const T &event)`            | Publish an event to all subscribers of type `T` |

---

## License

MIT
