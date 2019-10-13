# FindShaderc
# -------
#
# This will will define the following variables:
#
# SHADERC_FOUND    - true if Shaderc has been found
# SHADERC_GLSLC    - the glslc executable

find_program(Shaderc_GLSLC glslc)

set(shaderc_HEADER_SEARCH_DIRS
	"/usr/include"
	"/usr/local/include"
	"${CMAKE_SOURCE_DIR}/includes")
set(shaderc_LIB_SEARCH_DIRS
	"/usr/lib"
	"/usr/local/lib"
	"${CMAKE_SOURCE_DIR}/lib")

find_path(SHADERC_INCLUDE_DIR "shaderc/shaderc.hpp"
	PATHS ${shaderc_HEADER_SEARCH_DIRS})
find_library(SHADERC_LIBRARY
	NAMES shaderc_shared shaderc_combined
	PATHS ${shaderc_LIB_SEARCH_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shaderc
	DEFAULT_MSG
	Shaderc_GLSLC
	SHADERC_LIBRARY
	SHADERC_INCLUDE_DIR)
