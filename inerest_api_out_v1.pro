##-*-makefile-*-########################################################################################################
# Copyright 2021 Inesonic, LLC
# All Rights Reserved
########################################################################################################################

########################################################################################################################
# Basic build characteristics
#

TEMPLATE = lib

QT += core network
QT -= gui

CONFIG += static c++14

########################################################################################################################
# Inesonic Public includes
#

INCLUDEPATH += include
HEADERS = include/rest_api_out_v1_common.h \
          include/rest_api_out_v1_server.h \
          include/rest_api_out_v1_inesonic_rest_handler_base.h \
          include/rest_api_out_v1_inesonic_rest_handler.h \
          include/rest_api_out_v1_inesonic_binary_rest_handler.h \

########################################################################################################################
# Source files
#

SOURCES = source/rest_api_out_v1_server.cpp \
          source/rest_api_out_v1_inesonic_rest_handler_base.cpp \
          source/rest_api_out_v1_inesonic_rest_handler.cpp \
          source/rest_api_out_v1_inesonic_binary_rest_handler.cpp \

########################################################################################################################
# Libraries
#

INCLUDEPATH += $${INECRYPTO_INCLUDE}

########################################################################################################################
# Locate build intermediate and output products
#

TARGET = inerest_api_out_v1

CONFIG(debug, debug|release) {
    unix:DESTDIR = build/debug
    win32:DESTDIR = build/Debug
} else {
    unix:DESTDIR = build/release
    win32:DESTDIR = build/Release
}

OBJECTS_DIR = $${DESTDIR}/objects
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/rcc
UI_DIR = $${DESTDIR}/ui

