project(tinyxml2)
add_library(${PROJECT_NAME} tinyxml2.cpp tinyxml2.h)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
