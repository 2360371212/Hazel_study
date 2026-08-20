cmake_minimum_required(VERSION 3.20)
project(HazelWorkspace LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/EHsc)
endif()

# Keep final binaries separate from CMake/MSBuild intermediate files.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# spdlog is used as a header-only dependency, so it produces no library.
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)

# Build GLFW only as Hazel's static dependency.
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)


add_subdirectory(Hazel/vendor/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(Hazel/vendor/GLFW EXCLUDE_FROM_ALL)
add_subdirectory(Hazel/vendor/GLAD EXCLUDE_FROM_ALL)

# Dear ImGui does not provide a project CMake target. Keep its cloned source
# tree untouched and describe the files used by Hazel here instead.
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Hazel/vendor/imgui")

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
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

add_subdirectory(Hazel)
add_subdirectory(Sandbox)

# Make Sandbox the startup project in Visual Studio.
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Sandbox)
