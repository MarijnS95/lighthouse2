if(WIN32)
	# Use prebuilts on Windows:
	set(FREEIMAGE_ROOT "${CMAKE_SOURCE_DIR}/lib/FreeImage")
endif()
LIST(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/FreeImage")
find_package(FreeImage)
LIST(POP_FRONT CMAKE_MODULE_PATH)
