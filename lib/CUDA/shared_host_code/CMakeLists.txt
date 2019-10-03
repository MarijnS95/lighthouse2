project(CUDA_host_code LANGUAGES CXX)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
# Export as source library. All cpp files will be compiled in the context
# of the target "linking" against this.
add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE ${headers} ${sources})
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} INTERFACE nvrtc)

# TODO: Extract paths from OptiX installation
if(WIN32)
	target_compile_definitions(${PROJECT_NAME} INTERFACE OPTIX_6_INCLUDE_PATH="../../lib/OptiX/include/" OPTIX_INCLUDE_PATH="../../lib/OptiX7/include/")
else()
	target_compile_definitions(${PROJECT_NAME} INTERFACE OPTIX_6_INCLUDE_PATH="/opt/optix-6/include" OPTIX_INCLUDE_PATH="/opt/optix/include")
endif()
