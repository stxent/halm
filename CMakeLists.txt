# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

project(halm C)
cmake_minimum_required(VERSION 3.6)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build: Debug Release." FORCE)
endif()

option(USE_LTO "Enable Link Time Optimization." OFF)

set(PATH_XCORE "${PROJECT_SOURCE_DIR}/../xcore" CACHE STRING "Path to the XCORE library.")
set(CONFIG_FILE "${PROJECT_SOURCE_DIR}/.config" CACHE STRING "Path to the config file.")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(ParseConfig)
parse_config(FLAGS_LIBRARY "${CONFIG_FILE}")

# Default compiler flags
set(FLAGS_DEFAULT "-fdata-sections -ffunction-sections -Wall -Wextra -Wshadow -pedantic")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_DEFAULT} ${FLAGS_CPU} ${FLAGS_LIBRARY}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

if(${USE_LTO})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
endif()
if(DEFINED PLATFORM)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${PLATFORM}")
endif()

include_directories("${PATH_XCORE}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_subdirectory(core)
add_subdirectory(generic)
add_subdirectory(platform)
add_subdirectory(pm)
add_subdirectory(usb)

set(LIBRARY_OBJECTS $<TARGET_OBJECTS:platform>)
if(TARGET core)
    set(LIBRARY_OBJECTS ${LIBRARY_OBJECTS} $<TARGET_OBJECTS:core>)
endif()
if(TARGET generic)
    set(LIBRARY_OBJECTS ${LIBRARY_OBJECTS} $<TARGET_OBJECTS:generic>)
endif()
if(TARGET pm)
    set(LIBRARY_OBJECTS ${LIBRARY_OBJECTS} $<TARGET_OBJECTS:pm>)
endif()
if(TARGET usb)
    set(LIBRARY_OBJECTS ${LIBRARY_OBJECTS} $<TARGET_OBJECTS:usb>)
endif()

add_library(${PROJECT_NAME} ${LIBRARY_OBJECTS})
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
