# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

list(APPEND SOURCE_FILES "flash.c")
list(APPEND SOURCE_FILES "work_queue_default.c")

if(CONFIG_GENERIC_BUFFERING_PROXY)
    list(APPEND SOURCE_FILES "buffering_proxy.c")
endif()

if(CONFIG_GENERIC_GPIO_BUS)
    list(APPEND SOURCE_FILES "gpio_bus.c")
endif()

if(CONFIG_GENERIC_MMCSD)
    list(APPEND SOURCE_FILES "mmcsd.c")
endif()

if(CONFIG_GENERIC_SDIO_SPI)
    list(APPEND SOURCE_FILES "sdio_spi.c")
endif()

if(CONFIG_GENERIC_SOFTWARE_TIMER_32)
    list(APPEND SOURCE_FILES "software_timer_32.c")
endif()

if(CONFIG_GENERIC_SOFTWARE_TIMER_64)
    list(APPEND SOURCE_FILES "software_timer_64.c")
endif()

if(CONFIG_GENERIC_TIMER_FACTORY)
    list(APPEND SOURCE_FILES "timer_factory.c")
endif()

if(CONFIG_GENERIC_WQ)
    list(APPEND SOURCE_FILES "work_queue.c")
endif()

if(CONFIG_GENERIC_WQ_IRQ)
    list(APPEND SOURCE_FILES "work_queue_irq.c")
endif()

if(SOURCE_FILES)
    add_library(halm_generic OBJECT ${SOURCE_FILES})
endif()
