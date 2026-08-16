add_library(Hazel SHARED
    src/Hazel/Application.cpp
    src/Hazel/Log.cpp
)

target_include_directories(Hazel
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(Hazel
    PUBLIC
        HZ_PLATFORM_WINDOWS
    PRIVATE
        HZ_BUILD_DLL
)

target_link_libraries(Hazel
    PUBLIC
        spdlog::spdlog_header_only
)
