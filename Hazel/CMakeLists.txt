cmake_minimum_required(VERSION 3.20)
project(Hazel LANGUAGES CXX)

add_library(
    Hazel
    SHARED
    src/Hazel/Application.cpp
    src/Hazel/Log.cpp
)

target_include_directories(
    Hazel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
)

target_compile_definitions(
    Hazel
    PUBLIC HZ_PLATFORM_WINDOWS
    PRIVATE HZ_BUILD_DLL
)

if(MSVC)
    target_compile_options(Hazel
        PUBLIC
            /utf-8
    )
endif()