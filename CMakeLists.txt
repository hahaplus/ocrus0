cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (OCRus)

# Enable C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(/home/michael/ThirdPartLib/opencv-3.0.0/include)
link_directories(/home/michael/ThirdPartLib/opencv-3.0.0/lib)

add_subdirectory(src)
add_subdirectory(python)
