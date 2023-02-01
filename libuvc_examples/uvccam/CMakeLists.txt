cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(libuvc REQUIRED)
if (libuvc_FOUND)
	message(STATUS "libuvc found")
	message(STATUS "libuvc includes: " ${libuvc_INCLUDE_DIRS})
	message(STATUS "libuvc libraries: " ${libuvc_LIBRARIES})
endif (libuvc_FOUND)

project(uvccam)
add_library(uvccam uvccam.cpp)
target_link_libraries(uvccam PRIVATE ${libuvc_LIBRARIES})
target_include_directories(uvccam PRIVATE ${libuvc_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR})
