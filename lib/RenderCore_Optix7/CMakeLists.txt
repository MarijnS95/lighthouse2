project(RenderCore_Optix7 LANGUAGES CXX CUDA)

file(GLOB kernels "kernels/*.cu")
file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} SHARED ${sources} ${kernels} ${headers})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE COREDLL_EXPORTS=1)
target_link_libraries(${PROJECT_NAME} PRIVATE
	RenderSystem
	sharedBSDFs
	CUDA
	CUDA_host_code
	CUDA_kernel_code)

set(CMAKE_MODULE_PATH "/opt/optix/SDK/CMake/")
set(OptiX_INSTALL_DIR "/opt/optix/"
	CACHE PATH "Path to OptiX installed location."
	FORCE)
unset(OptiX_FOUND)
# find_package(OptiX 7.0.0 EXACT REQUIRED)
include("/opt/optix/SDK/CMake/FindOptiX.cmake")
# message(FATAL_ERROR "Optix 7 includes ${OptiX_INCLUDE}")
target_include_directories(${PROJECT_NAME} PRIVATE ${OptiX_INCLUDE})

# By default a CUDA-enabled project does not include the headers for non *.cu files
# nor link against the Driver API (libcuda)
# Instead of trying to compile these files with the CUDA compiler that adds the headers
# (And requires __host__ definitions etc...), include the header+libraries directly
# set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE CUDA)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
