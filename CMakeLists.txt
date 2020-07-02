# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

project(halm C)
cmake_minimum_required(VERSION 3.6)
cmake_policy(SET CMP0054 NEW)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build: Debug Release." FORCE)
endif()

option(USE_LTO "Enable Link Time Optimization." OFF)

set(PATH_XCORE "${PROJECT_SOURCE_DIR}/../xcore" CACHE STRING "Path to the XCORE library.")
set(HALM_CONFIG_FILE "${PROJECT_SOURCE_DIR}/.config" CACHE STRING "Path to the config file.")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(ParseConfig)
parse_config("${HALM_CONFIG_FILE}")

# Default compiler flags
set(FLAGS_DEFAULT "-fdata-sections -ffunction-sections -Wall -Wextra -Wshadow -pedantic")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_DEFAULT} ${FLAGS_CPU}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

if(${USE_LTO})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
endif()

if(DEFINED PLATFORM)
    set(DEFINITION_LIST ${DEFINITION_LIST} -D${PLATFORM})
elseif(UNIX)
    set(DEFINITION_LIST ${DEFINITION_LIST} -D_POSIX_C_SOURCE=200809L)
else()
    set(DEFINITION_LIST "")
endif()

set(TARGET_LIST core generic platform pm usb)
set(LIBRARY_OBJECTS "")

foreach(ENTRY ${TARGET_LIST})
    add_subdirectory(${ENTRY})
    if(TARGET halm_${ENTRY})
        set(LIBRARY_OBJECTS ${LIBRARY_OBJECTS} $<TARGET_OBJECTS:halm_${ENTRY}>)
        target_compile_definitions(halm_${ENTRY}
                PRIVATE ${DEFINITION_LIST}
        )
        target_include_directories(halm_${ENTRY}
                PRIVATE "${PATH_XCORE}/include"
                PRIVATE "${PROJECT_SOURCE_DIR}/include"
        )
    endif()
endforeach()

add_library(${PROJECT_NAME} ${LIBRARY_OBJECTS})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include")

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
