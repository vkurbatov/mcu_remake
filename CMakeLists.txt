cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "-O0 -ggdb3 ${CMAKE_C_FLAGS}" )
    set(CMAKE_CXX_FLAGS "-O0 -ggdb3 ${CMAKE_CXX_FLAGS}")
endif()


add_definitions(
    -DWEBRTC_POSIX
    )

set (CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(TARGET mcu_remake)

project(${TARGET})

set(SOURCES
    "main.cpp"
    )

set(HEADERS   

    )

include_directories(
                    liblargo
                    liblargo/core
                    )


add_executable(${TARGET}
               ${SOURCES}
               ${HEADERS}
                )

target_link_libraries(${TARGET}
                        asound
                        pthread
                        largo
                        )

add_subdirectory(liblargo)
