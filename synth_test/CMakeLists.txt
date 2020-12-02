cmake_minimum_required(VERSION 2.8)

set(TARGET synth_test)

project(${TARGET})

set(SOURCES
    "main.cpp"
    "synth_form.cpp"
    )

set(HEADERS   
    "synth_form.h"
    )

set(UIS
    "synth_form.ui"
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
                        asound
                        pthread
                        largo
                        lgcodec
                        ${Qt5Core_LIBRARIES}
                        ${Qt5Gui_LIBRARIES}
                        ${Qt5Widgets_LIBRARIES}
                        ${Qt5Multimedia_LIBRARIES}
                        )


