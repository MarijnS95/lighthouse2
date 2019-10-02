project(RenderCore_OptixPrime_B LANGUAGES CXX CUDA)

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

find_package(OptiX6 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE optix optix_prime)

# By default a CUDA-enabled project does not include the headers for non *.cu files
# nor link against the Driver API (libcuda)
# Instead of trying to compile these files with the CUDA compiler that adds the headers
# (And requires __host__ definitions etc...), include the header+libraries directly
# set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE CUDA)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
