# https://cmake.org/cmake/help/latest/module/FindCUDA.html
# 3.10 supports CUDA as first-class language:
cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0074 NEW)

project(Lighthouse2)

# Module and Config searches look in the cmake subdirectory first:
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_PREFIX_PATH}")

if(MSVC)
	# Consolidate all runtime libraries and executables to a single folder.
	# This has the following advantages:
	# - Single copy of external dependencies (except for those in lib/ for
	#   currently unused `install`able imported targets)
	# - Apps can find RenderCores directly, without PATH hacks
	# - Easier to run outside of Visual Studio, without needind a PATH or manually copying
	#   (working directory still needs to be relative to the assets: currently project root)

	set(OUTPUT_FOLDER "${PROJECT_SOURCE_DIR}/run" CACHE PATH "Output directory for executables and libraries (Windows only)")

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_FOLDER})
	set(CMAKE_PDB_OUTPUT_DIRECTORY ${OUTPUT_FOLDER})

	foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
		string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_FOLDER})
		set(CMAKE_PDB_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_FOLDER})
	endforeach()
endif()

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
	# Ensure rendercore is named just like the folder:
	if(NOT TARGET ${core_name})
		message(FATAL_ERROR "${core_name} Does not name a target!")
	endif()
	# Collect compiled rendercores for proper loading from apps:
	list(APPEND render_cores ${core_name})
endmacro()

include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
	# Allow constexpr usage on the device, without __device__ (eg. std::max):
	set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --expt-relaxed-constexpr")

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

message(STATUS "Found RenderCores: ${render_cores}")

foreach(core_name IN LISTS render_cores)
	list(APPEND core_folders $<TARGET_FILE_DIR:${core_name}>)
endforeach()

# Create a macro for app cmake definitions to call. Their folder name need not
# match their project/executable name.
function(register_app app_target)
	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13" AND UNIX)
		# Set rpath for dlopen to find the cores:
		target_link_directories(${PROJECT_NAME} PRIVATE ${core_folders})
	elseif(MSVC)
		# Ensure RenderCores are built as app dependency, in particular when
		# running (debugging) the startup project (an app) in Visual Studio.
		add_dependencies(${app_target} ${render_cores})

		# Note that target_link_directories has no effect, since libraries need either
		# be in the same directory, or on the PATH (CMAKE_MSVCIDE_RUN_PATH).
		# For now, all executables and libraries are compiled to the same
		# `/run` folder in the project root (only on WIN32).
	else()
		message(FATAL_ERROR "CMake < 3.13 doesn't support target_link_directories!")
		# TODO: Find a better solution on these platforms:
		# Also move all output to the same directory?
		# Use PATH variables during debugging?
		# Rely on install/export? (not efficient for debugging sessions)
	endif()
endfunction()

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

if(WIN32)
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ImguiApp)
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/data" DESTINATION share/)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/shaders" DESTINATION share/)
