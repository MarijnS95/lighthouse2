# FindShaderc
# -------
#
# This will find and expose the following Shaderc libraries:
# Shaderc::shaderc
# NOT YET:
# Shaderc::shaderc_static
# Shaderc::shaderc_combined
# Read https://github.com/google/shaderc/tree/master/libshaderc
# for more information.
#
# This will define the following variables:
#
# Shaderc_FOUND    - true if Shaderc has been found
# Shaderc_GLSLC    - the glslc executable
# Shaderc::Shaderc

# Keep in mind that these paths (to Windows-only libraries, for the time being)
# are searched _in addition to_ system directories.

if(TARGET Shaderc::shaderc)
	return()
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
	# Shaderc exposes these modulespecs in their PkgConfig definition:
	# shaderc:           Shader in a shared library, depends on other
	#                    shared libraries.
	# shaderc_static:    Shaderc in a static libary, depends on other
	#                    shared libraries.
	# shaderc_combined:  Shaderc and all dependencies in a single static library.
	# For a list of dependencies, see https://github.com/google/shaderc/tree/master/libshaderc#build-artifacts

	pkg_check_modules(Shaderc
		IMPORTED_TARGET
		shaderc)

	if(${Shaderc_FOUND}) # All modules are found
		# Expose the shared library. This relies on the OS
		# providing this libary as well as all dependencies.
		# TODO: Most safe might be
		pkg_check_modules(shaderc IMPORTED_TARGET GLOBAL shaderc)
		add_library(Shaderc::shaderc ALIAS PkgConfig::shaderc)

		# Done.
		return()
	endif()
endif(PkgConfig_FOUND)

find_program(Shaderc_GLSLC glslc
	PATHS
	"${CMAKE_SOURCE_DIR}/lib/vulkan/bin")

find_path(Shaderc_INCLUDE_DIR "shaderc/shaderc.hpp"
	PATHS
	"${CMAKE_SOURCE_DIR}/lib/vulkan/include")

find_library(Shaderc_SHARED_LIBRARY
	NAMES shaderc_shared
	PATHS
	"${CMAKE_SOURCE_DIR}/lib/vulkan/lib")

message(STATUS ${Shaderc_LIBRARY})

if(WIN32)
	find_file(Shaderc_SHARED_DLL shaderc_shared.dll
		PATHS
		"${CMAKE_SOURCE_DIR}/lib/vulkan/bin")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shaderc
	DEFAULT_MSG
	Shaderc_GLSLC
	Shaderc_SHARED_LIBRARY
	Shaderc_SHARED_DLL)

# TODO: Expose shared, combined and static targets in-line with PkgConfig
add_library(Shaderc::shaderc SHARED IMPORTED)
if(WIN32)
	set_target_properties(Shaderc::shaderc
		PROPERTIES
		IMPORTED_IMPLIB "${Shaderc_SHARED_LIBRARY}"
		IMPORTED_LOCATION "${Shaderc_SHARED_DLL}"
	)
else(WIN32)
	set_target_properties(Shaderc::shaderc
		PROPERTIES
		IMPORTED_LOCATION "${Shaderc_SHARED_LIBRARY}"
	)
endif(WIN32)
