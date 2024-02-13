cmake_minimum_required(VERSION 3.15)

project(graphiz)

find_package(raylib 5.0 REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/vertex.cpp
	src/edge.cpp
	src/util.cpp
)

target_link_libraries(${PROJECT_NAME} raylib)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
