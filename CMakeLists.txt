# https://cmake.org/cmake/help/latest/module/FindCUDA.html
# 3.10 supports CUDA as first-class language:
cmake_minimum_required(VERSION 3.10)

project(Lighthouse2)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/lib/FreeImage")

# External dependencies bundled with the source:
add_subdirectory("lib/glad")
add_subdirectory("lib/half2.1.0")
add_subdirectory("lib/tinyxml2")
add_subdirectory("lib/tinygltf")
add_subdirectory("lib/tinyobjloader")
add_subdirectory("lib/imgui")
add_subdirectory("lib/AntTweakBar")

# Platform configuration goes first:
add_subdirectory("lib/platform")
add_subdirectory("lib/RenderSystem")
# HACK! Circular dependency on project *headers*
target_include_directories(Platform PRIVATE "lib/RenderSystem")
# Note that there is a rather peculiar dependency mechanism. Things defined in system.h
# are implemented in platform.cpp, and headers are shared between lib/platform and
# lib/RenderSystem making dependency management ever so slightly less elegant.

# Include cross-compatible BSDF implementation:
add_subdirectory("lib/sharedBSDFs")

# Include CUDA support headers:
add_subdirectory("lib/CUDA")

# Include RenderCores:
file(GLOB cores "lib/RenderCore_*")
foreach(core IN LISTS cores)
	add_subdirectory(${core})
	get_filename_component(core_name ${core} NAME)
	# Collect target folders for runtime "linking" (rpath):
	list(APPEND core_folders $<TARGET_FILE_DIR:${core_name}>)
endforeach()

# Include apps:
file(GLOB app_directories "apps/*")
foreach(app IN LISTS app_directories)
	if(${app} MATCHES "app_matui$" AND NOT WIN32)
		# WARNING: Ignore anttweakbar implementation. This project
		# is deprecated and not worth building/including.
		continue()
	endif()
	add_subdirectory(${app})
endforeach()
