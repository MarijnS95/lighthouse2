project(imgui)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

# Create object library. This only bundles the compiled objects resulting
# from the source files, without doing any sort of linking.
# This is pretty much identical to including the source files directly in the
# target project, but in an abstracted way. Though, a static library would
# work just as well.
# Note that private link_libraries will be propagated to the targets linking
# against this object library.
add_library(${PROJECT_NAME} OBJECT ${sources} ${headers})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

# Ensure Imgui picks up GLAD, instead of picking one at random based on system header availability.
target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
