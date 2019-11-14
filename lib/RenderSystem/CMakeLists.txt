project(RenderSystem)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")
add_library(${PROJECT_NAME} ${sources} ${headers})
target_compile_definitions(${PROJECT_NAME} PRIVATE RENDERSYSTEMBUILD)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<BOOL:${MSVC}>:/arch:AVX2>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<BOOL:NOT ${MSVC}>:-mavx2 -mfma>)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
if(NOT WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()
if(MSVC)
	# Prevent CUDA projects from inheriting this flag.
	target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()

# External dependencies, PUBLIC because exposed through header:
target_link_libraries(${PROJECT_NAME} PUBLIC
	tinyxml2
	TinyGLTF::tinygltf
	TinyOBJLoader::tinyobjloader)

target_link_libraries(${PROJECT_NAME} PUBLIC Platform)

if(BUILD_SHARED_LIBS)
	install(TARGETS ${PROJECT_NAME})
endif()

