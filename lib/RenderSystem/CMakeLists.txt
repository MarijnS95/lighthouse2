project(RenderSystem)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} ${sources} ${headers})
target_compile_definitions(${PROJECT_NAME} PRIVATE RENDERSYSTEMBUILD)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
if(NOT WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()

# External dependencies, PUBLIC because exposed through header:
target_link_libraries(${PROJECT_NAME} PUBLIC
	tinyxml2
	TinyGLTF::tinygltf
	TinyOBJLoader::tinyobjloader)

target_link_libraries(${PROJECT_NAME} PUBLIC Platform)
