# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(${CONFIG_GENERIC_GPIO_BUS})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/gpio_bus.c")
endif()

if(${CONFIG_GENERIC_SDIO_SPI})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdio_spi.c")
endif()

if(${CONFIG_GENERIC_SDCARD})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdcard.c")
endif()

if(${CONFIG_GENERIC_SOFTWARE_TIMER})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/software_timer.c")
endif()

if(${CONFIG_GENERIC_WORK_QUEUE})
    list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/work_queue.c")
endif()

if(SOURCE_FILES)
    add_library(generic OBJECT ${SOURCE_FILES})
endif()
