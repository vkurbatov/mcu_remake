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
    "core/media/common/data_queue.cpp"
    "core/media/common/multipoint_data_queue.cpp"
    "core/media/common/media_slot.cpp"
    "core/media/common/media_point.cpp"
    "core/media/common/media_queue.cpp"
    "core/media/audio/audio_resampler.cpp"
    "core/media/audio/audio_point.cpp"
    "core/media/audio/audio_stream.cpp"
    "core/media/audio/audio_server.cpp"
    "core/media/audio/audio_slot.cpp"
    "core/media/audio/audio_composer.cpp"
    "core/media/audio/audio_mixer.cpp"
    "core/media/audio/volume_controller.cpp"
    "core/media/audio/channels/audio_channel.cpp"
    "core/media/audio/channels/alsa/alsa_channel.cpp"
    )

set(HEADERS   
    "core/core-tools/logging.h"
    "core/media/common/i_data_queue.h"
    "core/media/common/i_multipoint_data_queue.h"
    "core/media/common/i_media_queue.h"
    "core/media/common/i_media_slot.h"
    "core/media/common/i_media_point.h"
    "core/media/common/i_media_stream.h"
    "core/media/audio/i_data_collection.h"
    "core/media/audio/i_audio_formatter.h"
    "core/media/audio/i_audio_point.h"
    "core/media/audio/i_volume_controller.h"
    "core/media/audio/i_audio_stream.h"
    "core/media/audio/i_audio_server.h"
    "core/media/audio/i_audio_composer.h"
    "core/media/audio/i_audio_slot.h"
    "core/media/audio/i_audio_mixer.h"
    "core/media/audio/channels/i_audio_channel.h"
    "core/media/common/media_types.h"
    "core/media/common/data_queue.h"
    "core/media/common/multipoint_data_queue.h"
    "core/media/common/media_queue.h"
    "core/media/common/media_slot.h"
    "core/media/common/media_point.h"
    "core/media/audio/audio_resampler.h"
    "core/media/audio/audio_format.h"
    "core/media/audio/audio_point.h"
    "core/media/audio/volume_controller.h"
    "core/media/audio/audio_stream.h"
    "core/media/audio/audio_server.h"
    "core/media/audio/audio_slot.h"
    "core/media/audio/audio_composer.h"
    "core/media/audio/audio_mixer.h"
    "core/media/audio/channels/audio_channel.h"
    "core/media/audio/channels/audio_channel_params.h"
    "core/media/audio/channels/alsa/alsa_channel.h"
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
