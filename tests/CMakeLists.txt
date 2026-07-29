include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(Lumadi_Events_Tests
        signal_tests.cpp
        event_bus_tests.cpp
)

target_link_libraries(Lumadi_Events_Tests
        PRIVATE
        Lumadi_Events
        Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(Lumadi_Events_Tests)