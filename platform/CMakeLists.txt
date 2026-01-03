# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

if(${CMAKE_SYSTEM_NAME} STREQUAL "Generic")
    if(NOT DEFINED PLATFORM)
        message(FATAL_ERROR "PLATFORM not defined")
    endif()

    if(${PLATFORM} MATCHES "^BL")
        add_subdirectory(bouffalo)
    elseif(${PLATFORM} MATCHES "^IMXRT")
        add_subdirectory(imxrt)
    elseif(${PLATFORM} MATCHES "^LPC")
        add_subdirectory(lpc)
    elseif(${PLATFORM} MATCHES "^M03|^M48")
        add_subdirectory(numicro)
    elseif(${PLATFORM} MATCHES "^STM32")
        add_subdirectory(stm32)
    endif()
else()
    # Target system is not a bare-metal system, generic drivers are used
    add_subdirectory(generic)
endif()
