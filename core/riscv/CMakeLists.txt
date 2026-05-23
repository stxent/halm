# Copyright (C) 2026 xent
# Project is distributed under the terms of the MIT License

list(APPEND SOURCE_FILES "irq.c")
list(APPEND STARTUP_FILES "startup.c")

if(CONFIG_PM)
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(CONFIG_CORE_RISCV_FPU)
    list(APPEND SOURCE_FILES "fpu.c")
endif()

add_library(halm_core OBJECT ${STARTUP_FILES} ${SOURCE_FILES})
set_source_files_properties(${STARTUP_FILES} PROPERTIES COMPILE_OPTIONS "${FLAGS_STARTUP}")
