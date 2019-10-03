project(Platform)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} ${sources} ${headers})
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

find_package(FreeImage REQUIRED)
find_package(glfw3 REQUIRED)

if(WIN32)
	# Try to find a system version first (ZLIB_ROOT would take precedence)
	find_package(ZLIB QUIET)
	if(NOT ZLIB_FOUND)
		set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/lib/zlib")
	endif()
endif()

find_package(ZLIB REQUIRED)

# External dependencies, PUBLIC because exposed through header:
target_link_libraries(${PROJECT_NAME} PUBLIC
	glad
	glfw
	Half::half
	FreeImage::freeimage
	ZLIB::ZLIB)
