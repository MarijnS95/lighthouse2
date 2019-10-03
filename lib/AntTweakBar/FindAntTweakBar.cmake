if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITS "64")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

find_path(AntTweakBar_INCLUDE_DIR
	NAMES AntTweakBar.h
	PATHS "${CMAKE_CURRENT_LIST_DIR}/include"
)
find_library(AntTweakBar_LIBRARY
	NAMES AntTweakBar${BITS}
	PATHS "${CMAKE_CURRENT_LIST_DIR}/lib"
)
find_file(AntTweakBar_RUNTIME_LIBRARY
	NAMES AntTweakBar${BITS}.dll
	PATHS "${CMAKE_CURRENT_LIST_DIR}/lib"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AntTweakBar
	DEFAULT_MSG
	AntTweakBar_INCLUDE_DIR
	AntTweakBar_LIBRARY
	AntTweakBar_RUNTIME_LIBRARY
)

add_library(AntTweakBar::anttweakbar SHARED IMPORTED)
target_include_directories(AntTweakBar::anttweakbar INTERFACE ${AntTweakBar_INCLUDE_DIR})

set_target_properties(AntTweakBar::anttweakbar
	PROPERTIES
	IMPORTED_IMPLIB "${AntTweakBar_LIBRARY}"
	IMPORTED_LOCATION "${AntTweakBar_RUNTIME_LIBRARY}"
)
