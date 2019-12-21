cmake_minimum_required(VERSION 2.8)

set(TARGET video_test)

project(${TARGET})

set(SOURCES
    "main.cpp"
    "video_form.cpp"
    "yuv_video_buffer.cpp"
    "rgb_video_buffer.cpp"
    "video_surface.cpp"
    )

set(HEADERS
    "video_form.h"
    "yuv_video_buffer.h"
    "rgb_video_buffer.h"
    "video_surface.h"
    )

set(UIS
    "video_form.ui"
    )


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)

QT5_WRAP_UI( UI_HEADERS ${UIS} )

add_executable(${TARGET}
               ${SOURCES}
               ${HEADERS}
               ${UI_HEADERS}
               )

target_link_libraries(${TARGET}
                        pthread
                        largo
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Multimedia_LIBRARIES}
                        )


