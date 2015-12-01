cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (OCRus)

# Enable C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
add_subdirectory(python)
