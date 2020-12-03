# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    if(NOT DEFINED PLATFORM)
        message(FATAL_ERROR "PLATFORM not defined")
    endif()

    string(FIND ${PLATFORM} "LPC" PLATFORM_IS_LPC)
    string(FIND ${PLATFORM} "STM32" PLATFORM_IS_STM32)

    if(${PLATFORM_IS_LPC} EQUAL 0)
        add_subdirectory(lpc)
    elseif(${PLATFORM_IS_STM32} EQUAL 0)
        add_subdirectory(stm32)
    endif()
else()
    # Target system is not a bare-metal system, generic drivers are used
    add_subdirectory(generic)
endif()
