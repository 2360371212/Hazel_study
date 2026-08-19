add_library(Hazel SHARED
    src/Hazel/Application.cpp
    src/Hazel/Log.cpp
    src/Hazel/Layer.cpp
    src/Hazel/LayerStack.cpp
)

if(WIN32)
    target_sources(Hazel
        PRIVATE
            src/Platform/Windows/WindowsWindow.cpp
    )
else()
    message(FATAL_ERROR "Hazel currently only supports Windows")
endif()


target_include_directories(Hazel
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(Hazel
    PUBLIC
        HZ_PLATFORM_WINDOWS
    PRIVATE
        HZ_BUILD_DLL
        HZ_ENABLE_ASSERTS
)

find_package(OpenGL REQUIRED)

target_link_libraries(Hazel
    PUBLIC
        spdlog::spdlog_header_only
    PRIVATE
        glfw
        OpenGL::GL
)

target_precompile_headers(
    Hazel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/hzpch.h
) 
