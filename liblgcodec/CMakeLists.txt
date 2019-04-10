cmake_minimum_required(VERSION 2.8)

set(TARGET lgcodec)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(${TARGET})

set(SOURCES
    main.cpp
)

set(PRIVATE_HEADERS
    
)

set(PUBLIC_HEADERS
    i_codec.h
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
    rt
    fdk-aac

)

# install(TARGETS ${TARGET} LIBRARY DESTINATION ${INSTALL_LIBRARIES_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION "${LARGO_INCLUDE_DIR}/liblgcodec"
        FILES_MATCHING PATTERN "*.h"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
