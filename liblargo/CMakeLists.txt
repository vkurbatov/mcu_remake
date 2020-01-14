cmake_minimum_required(VERSION 2.8)

set(TARGET largo)

find_package(PkgConfig)

pkg_search_module(AVCODEC REQUIRED libavcodec>=57.89.100)
pkg_search_module(AVFORMAT REQUIRED libavformat>=57.71.100)
pkg_search_module(AVDEVICE REQUIRED libavdevice)
pkg_search_module(AVUTIL REQUIRED libavutil)
pkg_search_module(SWSCALE REQUIRED libswscale>=4.6.100)
pkg_search_module(V4L2 QUIET libv4l2>=0.8.8)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
    "core/media/common/media_format.cpp"
    "core/media/common/media_plane.cpp"
    "core/media/common/media_buffer.cpp"
    "core/media/common/media_frame.cpp"
    "core/media/common/media_frame_buffered.cpp"
    "core/media/common/ffmpeg/libav_base.cpp"
    "core/media/common/ffmpeg/libav_decoder.cpp"
    "core/media/common/ffmpeg/libav_stream_capturer.cpp"
    "core/media/common/ffmpeg/libav_converter.cpp"
    "core/media/common/v4l2/v4l2_base.cpp"
    "core/media/common/v4l2/v4l2_device.cpp"
    "core/media/common/v4l2/v4l2_api.cpp"
    "core/media/common/utils/format_converter.cpp"
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
    "core/media/audio/volume_calculator.cpp"
    "core/media/audio/audio_divider.cpp"
    "core/media/audio/channels/audio_channel.cpp"
    "core/media/audio/channels/audio_channel_params.cpp"
    "core/media/audio/channels/audio_channel_worker.cpp"
    "core/media/audio/channels/alsa/alsa_channel.cpp"
    "core/media/audio/channels/file/file_channel.cpp"
    "core/media/audio/audio_string_format_utils.cpp"
    "core/media/audio/tools/audio_event_server.cpp"
    "core/media/audio/tools/audio_processor.cpp"
    "core/media/video/video_frame_size.cpp"
    "core/media/video/video_frame_point.cpp"
    "core/media/video/video_frame_rect.cpp"
    "core/media/video/video_format.cpp"       
    "core/media/video/video_frame.cpp"
    "core/media/video/video_frame_buffered.cpp"
    "core/media/video/video_frame_builder.cpp"
    "core/media/video/video_frame_factory.cpp"
)

set(PRIVATE_HEADERS
    "core/media/common/media_types.h"
    "core/media/common/data_queue.h"
    "core/media/common/multipoint_data_queue.h"
    "core/media/common/media_queue.h"
    "core/media/common/media_slot.h"
    "core/media/common/media_point.h"
    "core/media/common/delay_timer.h"
    "core/media/common/guard_lock.h"
    "core/media/common/sync_point.h"
    "core/media/common/media_plane.h"
    "core/media/common/media_buffer.h"
    "core/media/common/media_frame.h"
    "core/media/common/media_frame_buffered.h"
    "core/media/common/ffmpeg/libav_base.h"
    "core/media/common/ffmpeg/libav_decoder.h"
    "core/media/common/ffmpeg/libav_stream_capturer.h"
    "core/media/common/ffmpeg/libav_converter.h"
    "core/media/common/v4l2/v4l2_base.h"
    "core/media/common/v4l2/v4l2_device.h"
    "core/media/common/v4l2/v4l2_api.h"
    "core/media/common/utils/format_converter.h"
    "core/media/audio/audio_resampler.h"
    "core/media/audio/audio_point.h"
    "core/media/audio/volume_controller.h"
    "core/media/audio/volume_calculator.h"
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
    "core/media/video/video_frame.h"
    "core/media/video/video_frame_buffered.h"
    "core/media/video/video_frame_factory.h"
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
    "core/media/common/media_format.h"
    "core/media/common/i_media_buffer.h"
    "core/media/common/i_media_plane.h"
    "core/media/common/i_media_frame.h"
    "core/media/common/i_process_state_notifier.h" 
    "core/media/audio/i_audio_formatter.h"
    "core/media/audio/i_audio_point.h"
    "core/media/audio/i_volume_controller.h"
    "core/media/audio/i_volume_calculator.h"
    "core/media/audio/i_volume_average.h"
    "core/media/audio/i_audio_stream.h"
    "core/media/audio/i_audio_server.h"
    "core/media/audio/i_audio_composer.h"
    "core/media/audio/i_audio_slot.h"
    "core/media/audio/i_audio_mixer.h"
    "core/media/audio/i_audio_processing.h"
    "core/media/audio/channels/i_audio_channel.h"
    "core/media/video/video_frame_size.h"
    "core/media/video/video_frame_point.h"
    "core/media/video/video_frame_rect.h"
    "core/media/video/video_format.h"
    "core/media/video/video_frame_builder.h"
    "core/media/video/i_video_frame.h"
    "core/media/video/i_video_frame_factory.h"
    "core/media/video/i_video_frame_builder.h"
)

include_directories(
    ${V4L2_INCLUDE_DIRS}
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
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVDEVICE_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${V4L2_LIBRARIES}
    swscale
)

# install(TARGETS ${TARGET} LIBRARY DESTINATION ${INSTALL_LIBRARIES_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION "${LARGO_INCLUDE_DIR}/liblargo"
        FILES_MATCHING PATTERN "*.h"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
