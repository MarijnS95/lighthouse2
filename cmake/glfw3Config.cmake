# Check if the system has GLFW3:
find_package(glfw3 QUIET MODULE)

if(NOT glfw3_FOUND)
	# Use prebuilt:
	list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/GLFW")
	find_package(glfw3 MODULE)
	list(POP_FRONT CMAKE_MODULE_PATH)
endif()

# Windows has trouble finding the prebuilt dll. (like any other)
# Provide it the static version instead.
# TODO: This can hopefully be addressed when target_link_directories
# works as intended, despite missing unix-like RPATH capabilities.
if(TARGET glfw-static AND WIN32)
	set(PLATFORM_GLFW glfw-static)
else()
	set(PLATFORM_GLFW glfw)
endif()
