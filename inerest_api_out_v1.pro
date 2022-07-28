##-*-makefile-*-########################################################################################################
# Copyright 2021 - 2022 Inesonic, LLC
#
# MIT License:
#   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
#   documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
#   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
#   permit persons to whom the Software is furnished to do so, subject to the following conditions:
#   
#   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
#   Software.
#   
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
#   OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
#   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

