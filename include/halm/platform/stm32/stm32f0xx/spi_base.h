/*
 * halm/platform/stm32/stm32f0xx/spi_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SPI_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_SPI_BASE_H_
#define HALM_PLATFORM_STM32_STM32F0XX_SPI_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for SPI. */
enum
{
  SPI1,

  /* SPI2 is not available on STM32F03x devices */
  SPI2
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_SPI_BASE_H_ */
