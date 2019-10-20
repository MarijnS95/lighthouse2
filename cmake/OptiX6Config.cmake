# A copy of FindOptiX.cmake resides in the local tree:
LIST(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/OptiX/CMake")
find_package(OptiX)
LIST(POP_FRONT CMAKE_MODULE_PATH)

# Extend libraries with include directories:
target_include_directories(optix INTERFACE ${OptiX_INCLUDE})
target_include_directories(optixu INTERFACE ${OptiX_INCLUDE})
target_include_directories(optix_prime INTERFACE ${OptiX_INCLUDE})

foreach(lib IN ITEMS optix optixu optix_prime)
	list(APPEND install_libs $<TARGET_PROPERTY:${lib},IMPORTED_LOCATION>)
	list(APPEND install_libs $<TARGET_PROPERTY:${lib},IMPORTED_LOCATION>.6.5.0) # HACK!
endforeach()

install(FILES
	${install_libs}
	TYPE LIB)
