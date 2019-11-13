project(CUDA_kernel_code LANGUAGES CXX CUDA)

file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE ${headers})
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})
