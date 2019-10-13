# FindShaderc
# -------
#
# This will will define the following variables:
#
# Shaderc_FOUND    - true if Shaderc has been found
# Shaderc_GLSLC    - the glslc executable
# Shaderc::shaderc - linkable target

if (TARGET Shaderc::shaderc)
	# Already found
	return()
endif()

find_program(Shaderc_GLSLC glslc)

set(shaderc_HEADER_SEARCH_DIRS
	"/usr/include"
	"/usr/local/include"
	"${CMAKE_SOURCE_DIR}/includes")
set(shaderc_LIB_SEARCH_DIRS
	"/usr/lib"
	"/usr/local/lib"
	"${CMAKE_SOURCE_DIR}/lib")

find_path(Shaderc_INCLUDE_DIR "shaderc/shaderc.hpp"
	PATHS ${shaderc_HEADER_SEARCH_DIRS})
find_library(Shaderc_LIBRARY
	NAMES shaderc_shared
	PATHS ${shaderc_LIB_SEARCH_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shaderc
	DEFAULT_MSG
	Shaderc_GLSLC
	Shaderc_LIBRARY
	Shaderc_INCLUDE_DIR)

add_library(Shaderc::shaderc SHARED IMPORTED GLOBAL)
target_include_directories(Shaderc::shaderc INTERFACE ${Shaderc_INCLUDE_DIR})

set_target_properties(Shaderc::shaderc
	PROPERTIES
	IMPORTED_LOCATION "${Shaderc_LIBRARY}")
