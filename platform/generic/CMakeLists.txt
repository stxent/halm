# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

include(FindPkgConfig)
pkg_check_modules(LIBUV REQUIRED libuv)

if(UNIX)
    set(CMAKE_SYSTEM_SOC "posix")
endif()

list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/console.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/mmf.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rtc.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/serial.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/udp.c")

add_library(platform OBJECT ${SOURCE_FILES})
