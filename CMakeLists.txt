cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

project (OCRus)

# Enable C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(cpp)

add_subdirectory(cpp)

install(CODE "execute_process(COMMAND python setup.py install 
			  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
