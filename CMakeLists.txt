cmake_minimum_required(VERSION 2.8)

project(mcu_remake)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "-O0 -ggdb3 ${CMAKE_C_FLAGS}" )
    set(CMAKE_CXX_FLAGS "-O0 -ggdb3 ${CMAKE_CXX_FLAGS}")
endif()

set (CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -Wall")

include_directories(
    "liblargo"
    "liblargo/core"
    "liblgcodec"
)

add_subdirectory(liblargo)
add_subdirectory(liblgcodec)
add_subdirectory(audio_test)
add_subdirectory(video_test)
