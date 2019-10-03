# Locate the glfw3 library
#
# This module defines the following variables:
#
# GLFW3_LIBRARY the name of the library;
# GLFW3_INCLUDE_DIR where to find glfw include files.
# GLFW3_FOUND true if both the GLFW3_LIBRARY and GLFW3_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you can define a
# variable called GLFW3_ROOT which points to the root of the glfw library
# installation.
#
# default search dirs
#
# Cmake file from: https://github.com/daw42/glslcookbook

set(_glfw3_HEADER_SEARCH_DIRS
		"C:/Program Files (x86)/glfw/include")
set(_glfw3_LIB_SEARCH_DIRS
		"C:/Program Files (x86)/glfw/lib-msvc110")

# Check environment for root search directory
set(_glfw3_ENV_ROOT $ENV{GLFW3_ROOT})
if(NOT GLFW3_ROOT AND _glfw3_ENV_ROOT)
	set(GLFW3_ROOT ${_glfw3_ENV_ROOT})
endif()

if(NOT GLFW3_ROOT)
	set(GLFW3_ROOT "${CMAKE_CURRENT_LIST_DIR}")
endif()

# Put user specified location at beginning of search
if(GLFW3_ROOT)
	list(PREPEND _glfw3_HEADER_SEARCH_DIRS "${GLFW3_ROOT}/include")
	list(PREPEND _glfw3_LIB_SEARCH_DIRS "${GLFW3_ROOT}/lib")
	list(PREPEND _glfw3_LIB_SEARCH_DIRS "${GLFW3_ROOT}/lib-vc2017")
endif()

# Search for the header
find_path(GLFW3_INCLUDE_DIR "GLFW/glfw3.h"
		PATHS ${_glfw3_HEADER_SEARCH_DIRS})

# Search for the library
find_library(GLFW3_LIBRARY NAMES glfw3 glfw
		PATHS ${_glfw3_LIB_SEARCH_DIRS})
if(WIN32)
	find_library(GLFW3_RUNTIME_LIBRARY NAMES glfw3dll
			PATHS ${_glfw3_LIB_SEARCH_DIRS})
	set(_fphsa_runtime_lib GLFW3_RUNTIME_LIBRARY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glfw3
	DEFAULT_MSG
	GLFW3_LIBRARY ${_fphsa_runtime_lib} GLFW3_INCLUDE_DIR)

if(WIN32 AND GLFW3_RUNTIME_LIBRARY)
	find_file(GLFW3_RUNTIME_DLL
		NAMES
		glfw3.dll
		PATHS ${_glfw3_LIB_SEARCH_DIRS}
	)
endif(WIN32 AND GLFW3_RUNTIME_LIBRARY)

if(glfw3_FOUND)
	add_library(glfw SHARED IMPORTED)
	target_include_directories(glfw INTERFACE ${GLFW3_INCLUDE_DIR})

	if(WIN32)
		set_target_properties(glfw
			PROPERTIES
			IMPORTED_IMPLIB "${GLFW3_RUNTIME_LIBRARY}"
			IMPORTED_LOCATION "${GLFW3_RUNTIME_DLL}"
		)
	else()
		set_target_properties(glfw
			PROPERTIES
			IMPORTED_LOCATION "${GLFW3_LIBRARY}"
		)
	endif()

	if(WIN32)
		add_library(glfw-static STATIC IMPORTED)
		target_include_directories(glfw-static INTERFACE ${GLFW3_INCLUDE_DIR})
		set_target_properties(glfw-static
			PROPERTIES
			IMPORTED_LOCATION "${GLFW3_LIBRARY}"
		)
	endif()
endif()
