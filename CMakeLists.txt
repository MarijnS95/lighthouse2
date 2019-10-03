# https://cmake.org/cmake/help/latest/module/FindCUDA.html
# 3.10 supports CUDA as first-class language:
cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0074 NEW)

project(Lighthouse2)

# Module and Config searches look in the cmake subdirectory first:
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_PREFIX_PATH}")

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

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

macro(add_render_core core_path)
	add_subdirectory(${core_path})
	get_filename_component(core_name ${core_path} NAME)
	if(NOT TARGET ${core_name})
		message(FATAL_ERROR "${core_name} Does not name a target!")
	endif()
	# Collect target folders for runtime "linking" (rpath):
	list(APPEND core_folders $<TARGET_FILE_DIR:${core_name}>)
endmacro()

include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
	# Include CUDA support headers:
	add_subdirectory("lib/CUDA")

	# Include CUDA+Optix RenderCores:
	file(GLOB cores "lib/RenderCore_Opti*" "lib/RenderCore_Prime*")
	foreach(core IN LISTS cores)
		add_render_core(${core})
	endforeach()
endif()

find_package(Vulkan QUIET)
find_package(Shaderc QUIET)
if(Vulkan_FOUND AND Shaderc_FOUND)
	add_render_core("lib/RenderCore_Vulkan_RT")
endif()

# Include default RenderCores:
add_render_core("lib/RenderCore_SoftRasterizer")

# Include applications:
file(GLOB app_directories "apps/*")
foreach(app IN LISTS app_directories)
	if(${app} MATCHES "app_matui$" AND NOT WIN32)
		# WARNING: Ignore anttweakbar implementation. This project
		# is deprecated and not worth building/including.
		continue()
	endif()
	add_subdirectory(${app})
endforeach()
