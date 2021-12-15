cmake_minimum_required(VERSION 3.10)

project(ssg)
project (TEST)
enable_testing()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(include)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "./src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "tests/*.cpp")
list(REMOVE_ITEM SOURCES "src/main.cpp")

add_executable(TEST ${SOURCES} ${TEST_SOURCES})
add_executable(ssg ${SOURCES} "src/main.cpp")

set_property(TARGET ssg PROPERTY CXX_STANDARD 20)
set_property(TARGET TEST PROPERTY CXX_STANDARD 20)


#Optional:
#target_link_libraries(ssg LLVM)
