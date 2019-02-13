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

set(WEBRTCAP_INC_ALSA "/usr/include/alsa/")

set(SOURCES
    "main.cpp"
    "core/media/audio/devices/alsa_device.cpp"
    "core/media/common/data_queue.cpp"
    "core/media/common/media_point.cpp"
    "core/media/audio/audio_formatter.cpp"
    "core/media/audio/audio_resampler.cpp"
    "core/media/audio/audio_point.cpp"
    )

set(HEADERS
    "core/media/audio/devices/alsa_device.h"
    "core/media/common/data_queue.h"
    "core/media/common/media_point.h"
    "core/media/audio/audio_formatter.h"
    "core/media/audio/audio_resampler.h"
    "core/media/audio/audio_format.h"
    "core/media/audio/audio_point.h"
    "core/core-tools/logging.h"
    )

include_directories(
                    "core"
                    )


add_executable(${TARGET}
               ${SOURCES}
               ${HEADERS}
                )

target_link_libraries(${TARGET}
                        asound
                        pthread
                        )
