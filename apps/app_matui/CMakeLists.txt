project(MatuiApp)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")
add_executable(${PROJECT_NAME} ${sources} ${headers})

# External dependencies:
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE AntTweakBar::anttweakbar)

# Internal dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)

# Set rpath for dlopen to find the cores:
target_link_directories(${PROJECT_NAME} PRIVATE ${core_folders})
