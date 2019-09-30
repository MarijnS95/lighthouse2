project(RenderCore_OptixPrime_B LANGUAGES CXX CUDA)

file(GLOB sources "*.cpp")
file(GLOB kernels "kernels/.cuda.cu")
file(GLOB headers "*.h")
add_library(${PROJECT_NAME} SHARED ${sources} ${kernels} ${headers})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE COREDLL_EXPORTS=1)
target_link_libraries(${PROJECT_NAME} PRIVATE
	RenderSystem
	sharedBSDFs
	CUDA
	CUDA_host_code)

# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "/opt/optix-6/SDK/CMake/")
set(OptiX_INSTALL_DIR "/opt/optix-6/"
	CACHE PATH "Path to OptiX installed location."
	FORCE)
# find_package(OptiX 6.5.0 EXACT REQUIRED)
# message(FATAL_ERROR "Optix 6 includes ${OptiX_INCLUDE}")
include("/opt/optix-6/SDK/CMake/FindOptiX.cmake")
# message(FATAL_ERROR ${OptiX_INCLUDE})
target_include_directories(${PROJECT_NAME} PRIVATE ${OptiX_INSTALL_DIR}/include) # ${OptiX_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE optix_prime)
target_link_libraries(${PROJECT_NAME} PRIVATE optix)

# By default a CUDA-enabled project does not include the headers for non *.cu files
# nor link against the Driver API (libcuda)
# Instead of trying to compile these files with the CUDA compiler that adds the headers
# (And requires __host__ definitions etc...), include the header+libraries directly
# set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE CUDA)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
