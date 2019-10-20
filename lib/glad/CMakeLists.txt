# Based on https://github.com/Dav1dde/glad/issues/186

cmake_minimum_required(VERSION 3.0)
project(glad)

add_library(${PROJECT_NAME} include/glad/glad.h src/glad.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)

if(BUILD_SHARED_LIBS)
	install(TARGETS ${PROJECT_NAME})
endif()
