add_executable(Sandbox
    src/SandboxApp.cpp
)

target_link_libraries(Sandbox
    PRIVATE
        Hazel::Hazel
)

set_target_properties(Sandbox PROPERTIES FOLDER "Applications")
