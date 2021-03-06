cmake_minimum_required(VERSION 2.8)

set(TARGET lgcodec)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(${TARGET})

set(SOURCES
    common/options.cpp
    common/options_helper.cpp
    common/bit_stream.cpp
    codec/common/media_packetizer.cpp
    codec/audio/audio_codec_utils.cpp
    codec/audio/audio_codec_options.cpp
    codec/audio/audio_codec.cpp
    codec/audio/libav/libav_wrapper.cpp
    codec/audio/libav/libav_audio_transcoder.cpp
    codec/audio/aac/aac_audio_transcoder.cpp
    codec/audio/aac/aac_audio_packetizer.cpp
    codec/audio/aac/aac_audio_codec_options.cpp
    codec/audio/aac/au_packetizer.cpp
)

set(PRIVATE_HEADERS
    common/options.h
    common/options_helper.h
    common/options_helper.tpp
    common/bit_stream.h
    common/bit_stream.tpp
    codec/common/media_packetizer.h
    codec/audio/audio_codec_utils.h
    codec/audio/audio_codec_options.h
    codec/audio/audio_codec.h
    codec/audio/libav/libav_wrapper.h
    codec/audio/libav/libav_audio_transcoder.h
    codec/audio/aac/aac_types.h
    codec/audio/aac/aac_audio_transcoder.h
    codec/audio/aac/aac_audio_packetizer.h
    codec/audio/aac/aac_audio_codec_options.h
    codec/audio/aac/au_types.h
    codec/audio/aac/au_packetizer.h
)

set(PUBLIC_HEADERS   
    codec/audio/audio_codec_types.h
    common/i_options.h
    codec/common/i_codec_module.h
    codec/common/i_codec_control.h
    codec/common/i_codec_transcoder.h
    codec/common/i_codec.h
    codec/common/i_packetizer.h
    codec/audio/i_audio_codec.h
)

include_directories(
)

add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

target_link_libraries(${TARGET} PRIVATE
    pthread
    avcodec
    avutil
    rt
#    fdk-aac

)

# install(TARGETS ${TARGET} LIBRARY DESTINATION ${INSTALL_LIBRARIES_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION "${LARGO_INCLUDE_DIR}/liblgcodec"
        FILES_MATCHING PATTERN "*.h"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
