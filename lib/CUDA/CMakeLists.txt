project(CUDA LANGUAGES CXX CUDA)

# file(GLOB headers "*.h") TODO
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})
