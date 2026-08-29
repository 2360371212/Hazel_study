# Automatically pick up engine implementation files. CONFIGURE_DEPENDS asks
# CMake to regenerate the build system when matching files are added or removed.
file(GLOB_RECURSE HAZEL_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hazel/*.cpp"
)

file(GLOB_RECURSE HAZEL_OPENGL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/OpenGL/*.cpp"
)

# ImGui's GLFW and OpenGL3 backends are already compiled by the imgui target in
# the workspace CMake file, so the old amalgamation source must not be compiled.
list(FILTER HAZEL_ENGINE_SOURCES EXCLUDE REGEX
    "[/\\\\]ImGui[/\\\\]ImGuiBuild\\.cpp$"
)

if(WIN32)
    file(GLOB_RECURSE HAZEL_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Windows/*.cpp"
    )
else()
    message(FATAL_ERROR "Hazel currently only supports Windows")
endif()

file(GLOB_RECURSE HAZEL_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

set(STB_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image")
set(STB_IMAGE_SOURCES
    "${STB_IMAGE_DIR}/stb_image.cpp"
    "${STB_IMAGE_DIR}/stb_image.h"
)

set(HAZEL_SOURCES
    ${HAZEL_ENGINE_SOURCES}
    ${HAZEL_OPENGL_SOURCES}
    ${HAZEL_PLATFORM_SOURCES}
)

add_library(Hazel STATIC
    ${HAZEL_SOURCES}
    ${HAZEL_HEADERS}
    ${STB_IMAGE_SOURCES}
)

add_library(Hazel::Hazel ALIAS Hazel)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES ${HAZEL_SOURCES} ${HAZEL_HEADERS}
)

source_group(TREE "${STB_IMAGE_DIR}"
    PREFIX "Dependencies/stb_image"
    FILES ${STB_IMAGE_SOURCES}
)

target_include_directories(Hazel
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_include_directories(Hazel SYSTEM
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm>"
    PRIVATE
        "$<BUILD_INTERFACE:${STB_IMAGE_DIR}>"
)

target_compile_definitions(Hazel
    PUBLIC
        HZ_STATIC
        HZ_PLATFORM_WINDOWS
        "$<$<CONFIG:Debug>:HZ_ENABLE_ASSERTS>"
    PRIVATE
        GLFW_INCLUDE_NONE
)

target_link_libraries(Hazel
    PUBLIC
        imgui::imgui
    PRIVATE
        glad::glad
        glfw
        OpenGL::GL
)

target_precompile_headers(Hazel
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/hzpch.h"
)

set_target_properties(Hazel PROPERTIES FOLDER "Engine")
