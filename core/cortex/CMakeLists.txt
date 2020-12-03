# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

set(SOURCE_PATHS "")
string(REPLACE "cortex-" "" CORE_VERSION "${CMAKE_SYSTEM_PROCESSOR}")

if(${CORE_VERSION} STREQUAL "m0")
    set(SOURCE_PATHS ${SOURCE_PATHS} "armv6m/*.c")
elseif(${CORE_VERSION} STREQUAL "m3")
    set(SOURCE_PATHS ${SOURCE_PATHS} "armv7m/*.c")
  elseif(${CORE_VERSION} MATCHES "m4|m7")
    set(SOURCE_PATHS ${SOURCE_PATHS} "armv7m/*.c" "armv7em/*.c")
endif()

file(GLOB SOURCE_FILES ${SOURCE_PATHS})

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(${CONFIG_CORE_CORTEX_SYSTICK})
    list(APPEND SOURCE_FILES "systick.c")
endif()

add_library(halm_core OBJECT ${SOURCE_FILES})
