cmake_minimum_required(VERSION 2.8)

set(TARGET audio_test)

project(${TARGET})

set(SOURCES
    "main.cpp"
    )

set(HEADERS   

    )

add_executable(${TARGET}
               ${SOURCES}
               ${HEADERS}
                )

target_link_libraries(${TARGET}
                        asound
                        pthread
                        largo
                        lgcodec
                        )


