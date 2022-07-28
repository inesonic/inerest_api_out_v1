##-*-cmake-*-###########################################################################################################
# Copyright 2016 - 2022 Inesonic, LLC
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

cmake_minimum_required(VERSION 3.16.3)
project(inerest_api_in_v1 LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 14)

find_package(Qt5 COMPONENTS Core)
find_package(Qt5 COMPONENTS Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Allow building either static or shared libraries
set(${PROJECT_NAME}_TYPE CACHE STRING "Static or shared library")
set_property(CACHE ${PROJECT_NAME}_TYPE PROPERTY STRINGS "SHARED;STATIC")
IF(NOT ${${PROJECT_NAME}_TYPE})
    SET(${PROJECT_NAME}_TYPE SHARED)
ENDIF()

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_TYPE}
            source/rest_api_out_v1_server.cpp
            source/rest_api_out_v1_inesonic_rest_handler_base.cpp
            source/rest_api_out_v1_inesonic_rest_handler.cpp
            source/rest_api_out_v1_inesonic_binary_rest_handler.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(${PROJECT_NAME} PUBLIC "include")
include_directories("include")

find_path(INECRYPTO_INCLUDE
          REQUIRED
          NAMES crypto_hmac.h crypto_helpers.h
          PATHS /usr/include/ /usr/local/include/ /opt/include/
)

include_directories(${INECRYPTO_INCLUDE})

target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Network)

find_library(INECRYPTO_LIB
             REQUIRED
             NAMES inecrypto
             PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64 /opt/lib ${INECRYPTO_LIBDIR}
)

target_link_libraries(${PROJECT_NAME} ${INECRYPTO_LIB})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

install(FILES include/rest_api_out_v1_common.h DESTINATION include)
install(FILES include/rest_api_out_v1_server.h DESTINATION include)
install(FILES include/rest_api_out_v1_inesonic_rest_handler_base.h DESTINATION include)
install(FILES include/rest_api_out_v1_inesonic_rest_handler.h DESTINATION include)
install(FILES include/rest_api_out_v1_inesonic_binary_rest_handler.h DESTINATION include)
