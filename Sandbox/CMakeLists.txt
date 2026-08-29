file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(Sandbox ${SANDBOX_SOURCES})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES ${SANDBOX_SOURCES}
)

target_link_libraries(Sandbox
    PRIVATE
        Hazel::Hazel
)

set_target_properties(Sandbox PROPERTIES FOLDER "Applications")
