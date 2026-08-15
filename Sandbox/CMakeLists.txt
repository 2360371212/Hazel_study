cmake_minimum_required(VERSION 3.20)
project(Sandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(
    HAZEL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Hazel/lib/Hazel.lib
)
set(
    HAZEL_DLL ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Hazel/bin/Hazel.dll
)

add_executable(Sandbox
    src/SandboxApp.cpp
)


target_include_directories(
    Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Hazel/src
)

target_link_libraries(Sandbox PRIVATE ${HAZEL_LIB})

target_compile_definitions(
    Sandbox PRIVATE HZ_PLATFORM_WINDOWS
)