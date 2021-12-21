cmake_minimum_required(VERSION 3.10)

project(whim)
project (TEST)
enable_testing()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(include)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "./src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "tests/*.cpp")
list(REMOVE_ITEM SOURCES "src/main.cpp")

add_executable(TEST ${SOURCES} ${TEST_SOURCES})
add_executable(whim ${SOURCES} "src/main.cpp")

set_property(TARGET whim PROPERTY CXX_STANDARD 20)
set_property(TARGET TEST PROPERTY CXX_STANDARD 20)

find_package(Threads REQUIRED)
target_link_libraries(whim Threads::Threads)
target_link_libraries(whim stdc++fs)
target_link_libraries(TEST Threads::Threads)
target_link_libraries(TEST stdc++fs)
