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

register_app(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if(BUILD_SHARED_LIBS)
	install(TARGETS ${PROJECT_NAME})
endif()

