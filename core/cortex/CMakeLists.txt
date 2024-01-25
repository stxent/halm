# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

set(SOURCE_FILES "")
string(REPLACE "cortex-" "" CORE_VERSION "${CMAKE_SYSTEM_PROCESSOR}")

if(${CORE_VERSION} MATCHES "m0|m0\+")
    list(APPEND SOURCE_FILES "armv6m/irq.c")
    list(APPEND SOURCE_FILES "armv6m/nvic.c")
    list(APPEND SOURCE_FILES "armv6m/spinlock.c")
elseif(${CORE_VERSION} MATCHES "m3|m4|m7")
    list(APPEND SOURCE_FILES "armv7m/irq.c")
    list(APPEND SOURCE_FILES "armv7m/nvic.c")
    list(APPEND SOURCE_FILES "armv7m/spinlock.c")

    if(CONFIG_CORE_CORTEX_FPU)
        list(APPEND SOURCE_FILES "armv7em/fpu.c")
    endif()
endif()

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(CONFIG_CORE_CORTEX_SYSTICK)
    list(APPEND SOURCE_FILES "systick.c")
endif()

add_library(halm_core OBJECT ${SOURCE_FILES})

if(DEFINED IRQ_THRESHOLD)
    target_compile_definitions(halm_core
            PUBLIC -DCONFIG_IRQ_THRESHOLD=${IRQ_THRESHOLD}
    )
endif()
