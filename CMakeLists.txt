cmake_minimum_required(VERSION 3.20)

project(HazelWorkspace LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
    add_compile_options(
        "$<$<COMPILE_LANGUAGE:CXX>:/EHsc>"
        "$<$<COMPILE_LANGUAGE:CXX>:/utf-8>"
        "$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>"
    )
endif()

# Keep final artifacts separate from CMake and compiler intermediate files.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# -----------------------------------------------------------------------------
# Third-party dependencies
# -----------------------------------------------------------------------------

# spdlog and GLM are header-only dependencies. Hazel declares their include
# directories directly, so they do not create separate CMake subprojects.

# GLFW: a static platform/windowing dependency.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
add_subdirectory(Hazel/vendor/GLFW EXCLUDE_FROM_ALL)

# GLAD already provides the glad::glad static target in its local CMake file.
add_subdirectory(Hazel/vendor/GLAD EXCLUDE_FROM_ALL)

# Dear ImGui has no upstream CMake target. Build its core and the two backends
# used by Hazel exactly once. Vendor sources remain untouched.
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Hazel/vendor/imgui")

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(imgui::imgui ALIAS imgui)

target_include_directories(imgui
    SYSTEM PUBLIC
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
)

target_compile_definitions(imgui
    PRIVATE
        GLFW_INCLUDE_NONE
)

target_link_libraries(imgui
    PRIVATE
        glfw
)

set_target_properties(glad glfw imgui PROPERTIES FOLDER "Dependencies")

find_package(OpenGL REQUIRED)

# -----------------------------------------------------------------------------
# Workspace targets
# -----------------------------------------------------------------------------

add_subdirectory(Hazel)
add_subdirectory(Sandbox)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT Sandbox
)
