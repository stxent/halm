# Copyright (C) 2021 xent
# Project is distributed under the terms of the MIT License

if(CONFIG_FAMILY_LPC43XX_M0_APP)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup_m0.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors_m0_app.c")
elseif(CONFIG_FAMILY_LPC43XX_M0_SUB)
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup_m0.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors_m0_sub.c")
else()
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/startup_m4.c")
    list(APPEND SOURCE_FILES "${CMAKE_SYSTEM_SOC}/vectors_m4.c")
endif()
