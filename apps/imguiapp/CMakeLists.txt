project(ImguiApp)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_executable(${PROJECT_NAME} ${sources} ${headers})

# External dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Internal dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)

register_app(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if(BUILD_SHARED_LIBS)
	install(TARGETS ${PROJECT_NAME})
endif()

