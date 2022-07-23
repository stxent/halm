# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

find_package(LibUV REQUIRED)
find_package(Threads REQUIRED)

if(UNIX)
    set(CMAKE_SYSTEM_SOC "posix")
endif()

if(CONFIG_PLATFORM_LINUX_CONSOLE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/console.c")
endif()

if(CONFIG_PLATFORM_LINUX_EVENT_QUEUE)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/event_queue.c")
endif()

if(CONFIG_PLATFORM_LINUX_MMF)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/mmf.c")
endif()

if(CONFIG_PLATFORM_LINUX_RTC)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/rtc.c")
endif()

if(CONFIG_PLATFORM_LINUX_SERIAL)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/serial.c")
endif()

if(CONFIG_PLATFORM_LINUX_SIGNAL_HANDLER)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/signal_handler.c")
endif()

if(CONFIG_PLATFORM_LINUX_TIMER)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/timer.c")
endif()

if(CONFIG_PLATFORM_LINUX_UDP)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/udp.c")
endif()

add_library(halm_platform OBJECT ${SOURCE_FILES})
target_link_libraries(halm_platform PUBLIC pthread uv)
