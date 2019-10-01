project(CUDA LANGUAGES CXX CUDA)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})

add_subdirectory("shared_host_code")
