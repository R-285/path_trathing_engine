cmake_minimum_required(VERSION 3.17)
project(path_tracing)

set(CMAKE_CXX_STANDARD 17)

set(${PROJECT_NAME}_SOURCE_FILES
        RenderTools.h
        RenderTools.cpp
        )

# Packages

find_package(OpenCV REQUIRED)
message("${CMAKE_SYSTEM_PREFIX_PATH}")
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(path_tracing main.cpp ${${PROJECT_NAME}_SOURCE_FILES})

target_link_libraries(path_tracing PRIVATE opencv_core opencv_highgui)