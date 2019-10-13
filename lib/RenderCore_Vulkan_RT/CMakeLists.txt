project("RenderCore_Vulkan_RT")

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")
add_library(${PROJECT_NAME} SHARED ${sources} ${kernels} ${headers})
# target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE COREDLL_EXPORTS=1)
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(Shaderc REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan Shaderc::shaderc)
