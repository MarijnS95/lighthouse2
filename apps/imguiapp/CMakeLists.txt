project(ImguiApp)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_executable(${PROJECT_NAME} ${sources} ${headers})

# External dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)
