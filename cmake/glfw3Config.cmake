# Check if the system has GLFW3:
find_package(glfw3 QUIET MODULE)

if(NOT glfw3_FOUND)
	# Use prebuilt:
	list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/GLFW")
	find_package(glfw3 MODULE)
	list(POP_FRONT CMAKE_MODULE_PATH)
endif()
