project(CUDA_host_code LANGUAGES CXX)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE ${headers} ${sources})
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} INTERFACE nvrtc)
