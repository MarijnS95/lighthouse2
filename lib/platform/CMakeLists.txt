project(Platform)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")
add_library(${PROJECT_NAME} ${sources} ${headers})
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

find_package(FreeImage REQUIRED)
find_package(glfw3 REQUIRED)

# External dependencies, PUBLIC because exposed through header:
target_link_libraries(${PROJECT_NAME} PUBLIC
	glad
	glfw
	Half::half
	FreeImage::freeimage)
