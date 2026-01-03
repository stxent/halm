# Copyright (C) 2026 xent
# Project is distributed under the terms of the MIT License

set(SOURCE_FILES "")
list(APPEND SOURCE_FILES "irq.c")
list(APPEND SOURCE_FILES "startup.c")

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(CONFIG_CORE_RISCV_FPU)
    list(APPEND SOURCE_FILES "fpu.c")
endif()

add_library(halm_core OBJECT ${SOURCE_FILES})
