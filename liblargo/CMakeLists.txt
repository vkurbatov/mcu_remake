cmake_minimum_required(VERSION 2.8)

set(TARGET largo)

project(${TARGET})

set(SOURCES
    "core/media/common/data_queue.cpp"
    "core/media/common/multipoint_data_queue.cpp"
    "core/media/common/media_slot.cpp"
    "core/media/common/media_point.cpp"
    "core/media/common/media_queue.cpp"
    "core/media/common/delay_timer.cpp"
    "core/media/common/guard_lock.cpp"
    "core/media/common/sync_point.cpp"
    "core/media/audio/audio_resampler.cpp"
    "core/media/audio/audio_format.cpp"
    "core/media/audio/audio_point.cpp"
    "core/media/audio/audio_stream.cpp"
    "core/media/audio/audio_server.cpp"
    "core/media/audio/audio_slot.cpp"
    "core/media/audio/audio_composer.cpp"
    "core/media/audio/audio_mixer.cpp"
    "core/media/audio/audio_dispatcher.cpp"
    "core/media/audio/audio_mux.cpp"
    "core/media/audio/audio_queue.cpp"
    "core/media/audio/volume_controller.cpp"
    "core/media/audio/audio_divider.cpp"
    "core/media/audio/channels/audio_channel.cpp"
    "core/media/audio/channels/audio_channel_params.cpp"
    "core/media/audio/channels/audio_channel_worker.cpp"
    "core/media/audio/channels/alsa/alsa_channel.cpp"
    "core/media/audio/channels/file/file_channel.cpp"
    "core/media/audio/audio_string_format_utils.cpp"
    "core/media/audio/tools/audio_event_server.cpp"
    "core/media/audio/tools/audio_processor.cpp"
)

set(PRIVATE_HEADERS
    "core/core-tools/logging.h"
    "core/media/common/media_types.h"
    "core/media/common/data_queue.h"
    "core/media/common/multipoint_data_queue.h"
    "core/media/common/media_queue.h"
    "core/media/common/media_slot.h"
    "core/media/common/media_point.h"
    "core/media/common/delay_timer.h"
    "core/media/common/guard_lock.h"
    "core/media/common/sync_point.h"
    "core/media/audio/audio_resampler.h"
    "core/media/audio/audio_point.h"
    "core/media/audio/volume_controller.h"
    "core/media/audio/audio_stream.h"
    "core/media/audio/audio_server.h"
    "core/media/audio/audio_slot.h"
    "core/media/audio/audio_composer.h"
    "core/media/audio/audio_mixer.h"
    "core/media/audio/audio_dispatcher.h"
    "core/media/audio/audio_mux.h"
    "core/media/audio/audio_queue.h"
    "core/media/audio/audio_divider.h"
    "core/media/audio/channels/audio_channel.h"
    "core/media/audio/channels/audio_channel_params.h"
    "core/media/audio/channels/audio_channel_worker.h"
    "core/media/audio/channels/alsa/alsa_channel.h"
    "core/media/audio/channels/file/file_channel.h"
    "core/media/audio/audio_string_format_utils.h"
    "core/media/audio/tools/audio_event_server.h"
    "core/media/audio/tools/audio_processor.h"
)

set(PUBLIC_HEADERS
    "core/media/audio/audio_format.h"
    "core/media/audio/channels/audio_channel_params.h"
    "core/media/common/i_data_queue.h"
    "core/media/common/i_multipoint_data_queue.h"
    "core/media/common/i_media_queue.h"
    "core/media/common/i_media_slot.h"
    "core/media/common/i_media_point.h"
    "core/media/common/i_media_stream.h"
    "core/media/common/i_data_collection.h"
    "core/media/common/i_sync_point.h"
    "core/media/audio/i_audio_formatter.h"
    "core/media/audio/i_audio_point.h"
    "core/media/audio/i_volume_controller.h"
    "core/media/audio/i_audio_stream.h"
    "core/media/audio/i_audio_server.h"
    "core/media/audio/i_audio_composer.h"
    "core/media/audio/i_audio_slot.h"
    "core/media/audio/i_audio_mixer.h"
    "core/media/audio/i_audio_processing.h"
    "core/media/audio/channels/i_audio_channel.h"
)

include_directories(
    "core"
)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

target_link_libraries(${TARGET} PRIVATE
    pthread
    rt
    asound
)
