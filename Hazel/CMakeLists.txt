set(HAZEL_SOURCES
    src/Hazel/Application.cpp
    src/Hazel/ImGui/ImGuiLayer.cpp
    src/Hazel/Layer.cpp
    src/Hazel/LayerStack.cpp
    src/Hazel/Log.cpp
    src/Hazel/Renderer/Shader.cpp
    src/Platform/OpenGL/OpenGLContext.cpp
    src/Hazel/Renderer/Buffer.cpp
    src/Platform/OpenGL/OpenGLBuffer.cpp
    src/Hazel/Renderer/VertexArray.cpp
    src/Platform/OpenGL/OpenGLVertexArray.cpp
    src/Hazel/Renderer/Renderer.cpp
    src/Hazel/Renderer/RendererAPI.cpp
    src/Hazel/Renderer/RenderCommand.cpp
    src/Platform/OpenGL/OpenGLRendererAPI.cpp
    src/Hazel/Renderer/OrthographicCamera.cpp
)

# ImGuiBuild.cpp is intentionally excluded. ImGui's GLFW and OpenGL3 backend
# sources are compiled once by the imgui target in the workspace CMake file.
if(WIN32)
    list(APPEND HAZEL_SOURCES
        src/Platform/Windows/WindowsInput.cpp
        src/Platform/Windows/WindowsWindow.cpp
    )
else()
    message(FATAL_ERROR "Hazel currently only supports Windows")
endif()

file(GLOB_RECURSE HAZEL_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE SPDLOG_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/spdlog/*.h"
)

file(GLOB_RECURSE GLM_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm/glm/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm/glm/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm/glm/*.inl"
)

add_library(Hazel STATIC
    ${HAZEL_SOURCES}
    ${HAZEL_HEADERS}
    ${SPDLOG_HEADERS}
    ${GLM_HEADERS}
)

add_library(Hazel::Hazel ALIAS Hazel)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES ${HAZEL_SOURCES} ${HAZEL_HEADERS}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include"
    PREFIX "Dependencies/spdlog"
    FILES ${SPDLOG_HEADERS}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
    PREFIX "Dependencies/glm"
    FILES ${GLM_HEADERS}
)

target_include_directories(Hazel
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_include_directories(Hazel
    SYSTEM PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm>"
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
