# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

string(REPLACE "cortex-" "" CPU_CORE "${CMAKE_SYSTEM_PROCESSOR}")

list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${CPU_CORE}/irq.c")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${CPU_CORE}/nvic.c")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${CPU_CORE}/spinlock.c")

if(${CONFIG_FPU})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${CPU_CORE}/fpu.c")
endif()

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pm.c")
endif()

if(${CONFIG_CORE_CORTEX_SYSTICK})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/systick.c")
endif()

add_library(core OBJECT ${SOURCE_FILES})
