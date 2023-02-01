cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LIBDRM REQUIRED libdrm)
# message(STATUS "LIBDRM display mode enabled")
# message(STATUS ${LIBDRM_INCLUDE_DIRS})

find_package(libuvc REQUIRED)
if (libuvc_FOUND)
	message(STATUS "libuvc found")
	message(STATUS "libuvc includes: " ${libuvc_INCLUDE_DIRS})
	message(STATUS "libuvc libraries: " ${libuvc_LIBRARIES})
endif (libuvc_FOUND)

set(LIBDRM_INCLUDE_DIRS /usr/include/libdrm)
set(PREVIEW_INCLUDE_DIRECTORY /usr/local/include)
set(PREVIEW_LIBRARIES /usr/local/lib/arm-linux-gnueabihf/libpreview.so)

# project(libuvc_simple)
# add_executable(simple_read simple_read.cpp)
# target_link_libraries(simple_read ${libuvc_LIBRARIES})
# target_include_directories(simple_read PRIVATE ${libuvc_INCLUDE_DIRS})

# project(drm_stream)
# add_executable(drm_stream drm_stream.cpp)
# target_link_libraries(drm_stream ${libuvc_LIBRARIES} ${PREVIEW_LIBRARIES})
# target_include_directories(drm_stream PRIVATE ${LIBDRM_INCLUDE_DIRS} ${libuvc_INCLUDE_DIRS} ${PREVIEW_INCLUDE_DIRECTORY})

add_subdirectory(uvccam)

project(libuvccam_test CXX C)
add_executable(libuvccam_test libuvccam_test.cpp)
target_link_libraries(libuvccam_test uvccam ${libuvc_LIBRARIES} ${PREVIEW_LIBRARIES})
target_include_directories(libuvccam_test PRIVATE ${CMAKE_SOURCE_DIR} ${LIBDRM_INCLUDE_DIRS} ${libuvc_INCLUDE_DIRS} ${PREVIEW_INCLUDE_DIRECTORY})