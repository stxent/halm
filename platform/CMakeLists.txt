# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    if(NOT DEFINED PLATFORM)
        message(FATAL_ERROR "PLATFORM not defined")
    endif()

    string(FIND ${PLATFORM} "LPC" PLATFORM_IS_NXP)
    string(FIND ${PLATFORM} "STM32" PLATFORM_IS_STM)

    if(${PLATFORM_IS_NXP} EQUAL 0)
        add_subdirectory(nxp)
    elseif(${PLATFORM_IS_STM} EQUAL 0)
        add_subdirectory(stm)
    endif()
else()
    # Target system is not a bare-metal system, generic drivers are used
    add_subdirectory(generic)
endif()
