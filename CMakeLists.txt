cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (OCRus)

# Enable C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Turn on more gcc warnings
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wno-long-long -pedantic -ansi")

# For Chen San
if(EXISTS "/home/michael/ThirdPartLib/opencv-3.0.0")
    include_directories(/home/michael/ThirdPartLib/opencv-3.0.0/include)
    link_directories(/home/michael/ThirdPartLib/opencv-3.0.0/lib)
endif()

add_subdirectory(src)
add_subdirectory(python)
