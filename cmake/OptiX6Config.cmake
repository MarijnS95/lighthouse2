# A copy of FindOptiX.cmake resides in the local tree:
LIST(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/OptiX/CMake")
find_package(OptiX)
LIST(POP_FRONT CMAKE_MODULE_PATH)

# Extend libraries with include directories:
target_include_directories(optix INTERFACE ${OptiX_INCLUDE})
target_include_directories(optixu INTERFACE ${OptiX_INCLUDE})
target_include_directories(optix_prime INTERFACE ${OptiX_INCLUDE})
