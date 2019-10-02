# Create CACHE PATH for OptiX 7 installation, defaulting to the bundled
# OptiX7 headers and Windows binaries in this project:
set(OptiX7_INSTALL_DIR "${CMAKE_SOURCE_DIR}/lib/OptiX7/"
	CACHE PATH "Path to OptiX7 installed location.")

# OptiX7 is a header-only library. Instead of reusing their FindOptiX.cmake
# that conflicts with OptiX 6 and doesn't handle find_package args properly,
# find the include folder and use FindPackageHandleStandardArgs

find_path(OptiX7_INCLUDE
	NAMES optix.h
	PATHS "${OptiX7_INSTALL_DIR}/include/")

# Handle QUIET & REQUIRED flags. Set OptiX7_FOUND properly:
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OptiX7
	DEFAULT_MSG
	OptiX7_INCLUDE)

# Create linkable header-only library. Note that this library is _not_ GLOBAL
# because that clashes with OptiX6. Every project needing OptiX7 can conveniently
# call find_package(OptiX7 [REQUIRED])
add_library(optix INTERFACE IMPORTED)
target_include_directories(optix INTERFACE ${OptiX7_INCLUDE})
