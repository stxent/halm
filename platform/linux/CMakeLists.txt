#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

set(SOURCE_FILES "")
set(SOURCE_FILES_LIST "")

set(CMAKE_SYSTEM_SOC "generic")
set(CMAKE_SYSTEM_VENDOR "linux")

list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/console.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/mmf.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rtc.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/serial.c")
list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/udp.c")

prepend(SOURCE_FILES_LIST "${PROJECT_SOURCE_DIR}/platform/linux" ${SOURCE_FILES})
list(APPEND LIBRARY_SOURCES "${SOURCE_FILES_LIST}")
