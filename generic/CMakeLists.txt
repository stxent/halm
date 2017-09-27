#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

set(SOURCE_FILES "")
set(SOURCE_FILES_LIST "")

if(${CONFIG_GENERIC_GPIO_BUS})
    list(APPEND SOURCE_FILES "gpio_bus.c")
endif()

if(${CONFIG_GENERIC_SDIO_SPI})
    list(APPEND SOURCE_FILES "sdio_spi.c")
endif()

if(${CONFIG_GENERIC_SDCARD})
    list(APPEND SOURCE_FILES "sdcard.c")
endif()

if(${CONFIG_GENERIC_SOFTWARE_TIMER})
    list(APPEND SOURCE_FILES "software_timer.c")
endif()

if(${CONFIG_GENERIC_WORK_QUEUE})
    list(APPEND SOURCE_FILES "work_queue.c")
endif()

prepend(SOURCE_FILES_LIST "${PROJECT_SOURCE_DIR}/generic" ${SOURCE_FILES})
list(APPEND LIBRARY_SOURCES ${SOURCE_FILES_LIST})
