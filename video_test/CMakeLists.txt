cmake_minimum_required(VERSION 2.8)

set(TARGET video_test)

project(${TARGET})

set(SOURCES
    "main.cpp"
    )

set(MOC_HEADERS
    "mainwindow.h"
    )
set(HEADERS
    )

set(UIS
    "mainwondow.ui"
    )

set ( RESOURCES
# main.qrc
)

# Next lines needed for building all Qt projects
find_package( Qt4 REQUIRED )
include( ${QT_USE_FILE} )
add_definitions( ${QT_DEFINITIONS} )
include_directories( ${CMAKE_BINARY_DIR} )

# using Qt meta-system (precompiler)
QT4_ADD_RESOURCES( RES_SOURCES ${RESOURCES} )
QT4_WRAP_UI( UI_HEADERS ${UIS} )
QT4_WRAP_CPP( MOC_SRCS ${MOC_HEADERS} )

add_executable(${TARGET}
               ${SOURCES}
               ${HEADERS}
                )

target_link_libraries(${TARGET}
                        pthread
                        largo
                        )


