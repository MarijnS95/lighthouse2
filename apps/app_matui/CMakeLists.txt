project(MatuiApp)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")
add_executable(${PROJECT_NAME} ${sources} ${headers})

# External dependencies:
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_GLFW})
find_package(AntTweakBar REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE AntTweakBar::anttweakbar)

# Internal dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)

# Set rpath for dlopen to find the cores:
if(${CMAKE_VERSION} VERSION_LESS "3.13")
	message(WARNING "CMake < 3.13 doesn't support target_link_directories!")
else()
	target_link_directories(${PROJECT_NAME} PRIVATE ${core_folders})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
