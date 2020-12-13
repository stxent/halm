/*
 * halm/platform/stm32/stm32f1xx/spi_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SPI_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_SPI_BASE_H_
#define HALM_PLATFORM_STM32_STM32F1XX_SPI_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for SPI. */
enum
{
  SPI1,
  SPI2,

  /* Available on STM32F105 and STM32F107 devices only */
  SPI3
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_SPI_BASE_H_ */
