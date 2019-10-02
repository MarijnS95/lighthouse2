project(ImguiApp)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_executable(${PROJECT_NAME} ${sources} ${headers})

# External dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Internal dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)

# Set rpath for dlopen to find the cores:
target_link_directories(${PROJECT_NAME} PRIVATE ${core_folders})
