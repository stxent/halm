# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

string(REPLACE "cortex-" "" CPU_CORE "${CMAKE_SYSTEM_PROCESSOR}")

list(APPEND SOURCE_FILES "${CPU_CORE}/irq.c")
list(APPEND SOURCE_FILES "${CPU_CORE}/nvic.c")
list(APPEND SOURCE_FILES "${CPU_CORE}/spinlock.c")

if(${CONFIG_FPU})
    list(APPEND SOURCE_FILES "${CPU_CORE}/fpu.c")
endif()

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(${CONFIG_CORE_CORTEX_SYSTICK})
    list(APPEND SOURCE_FILES "systick.c")
endif()

add_library(core OBJECT ${SOURCE_FILES})
