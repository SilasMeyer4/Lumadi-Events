cmake_minimum_required(VERSION 3.20)
project(Lumadi_Events)

include(CTest)

add_library(Lumadi_Events INTERFACE
        tests/signal_tests.cpp
)

target_compile_features(Lumadi_Events INTERFACE cxx_std_17)


target_include_directories(Lumadi_Events
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif ()
