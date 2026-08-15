cmake_minimum_required(VERSION 3.20)
project(Hazel LANGUAGES CXX)

add_library(
    Hazel
    SHARED
    src/Hazel/Application.cpp
)

target_include_directories(
    Hazel
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
)

target_compile_definitions(
    Hazel
    PUBLIC HZ_PLATFORM_WINDOWS
    PRIVATE HZ_BUILD_DLL
)