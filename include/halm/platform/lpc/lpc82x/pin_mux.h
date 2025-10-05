/*
 * halm/platform/lpc/lpc82x/pin_mux.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC82X_PIN_MUX_H_
#define HALM_PLATFORM_LPC_LPC82X_PIN_MUX_H_
/*----------------------------------------------------------------------------*/
#define PINMUX_I2C_STRIDE  2
#define PINMUX_SPI_STRIDE  7
#define PINMUX_UART_STRIDE 5

enum [[gnu::packed]] PinFixedMuxIndex
{
	PINMUX_FIXED_ACMP_I1  = 0,  /* PIO0_0 */
	PINMUX_FIXED_ACMP_I2  = 1,  /* PIO0_1 */
	PINMUX_FIXED_ACMP_I3  = 2,  /* PIO0_14 */
	PINMUX_FIXED_ACMP_I4  = 3,  /* PIO0_23 */
	PINMUX_FIXED_SWCLK    = 4,  /* PIO0_3 */
	PINMUX_FIXED_SWDIO    = 5,  /* PIO0_2 */
	PINMUX_FIXED_XTALIN   = 6,  /* PIO0_8 */
	PINMUX_FIXED_XTALOUT  = 7,  /* PIO0_9 */
	PINMUX_FIXED_RST      = 8,  /* PIO0_5 */
	PINMUX_FIXED_CLKIN    = 9,  /* PIO0_1 */
	PINMUX_FIXED_VDDCMP   = 10, /* PIO0_6 */
	PINMUX_FIXED_I2C0_SDA = 11, /* PIO0_11 */
	PINMUX_FIXED_I2C0_SCL = 12, /* PIO0_10 */
	PINMUX_FIXED_ADC0     = 13, /* PIO0_7 */
	PINMUX_FIXED_ADC1     = 14, /* PIO0_6 */
	PINMUX_FIXED_ADC2     = 15, /* PIO0_14 */
	PINMUX_FIXED_ADC3     = 16, /* PIO0_23 */
	PINMUX_FIXED_ADC4     = 17, /* PIO0_22 */
	PINMUX_FIXED_ADC5     = 18, /* PIO0_21 */
	PINMUX_FIXED_ADC6     = 19, /* PIO0_20 */
	PINMUX_FIXED_ADC7     = 20, /* PIO0_19 */
	PINMUX_FIXED_ADC8     = 21, /* PIO0_18 */
	PINMUX_FIXED_ADC9     = 22, /* PIO0_17 */
	PINMUX_FIXED_ADC10    = 23, /* PIO0_13 */
	PINMUX_FIXED_ADC11    = 24, /* PIO0_4 */

  PINMUX_FIXED_END
};

enum [[gnu::packed]] PinMuxIndex
{
  PINMUX_UNDEFINED      = -1,

  PINMUX_UART0_TXD      = 0,
  PINMUX_UART0_RXD      = 1,
  PINMUX_UART0_RTS      = 2,
  PINMUX_UART0_CTS      = 3,
  PINMUX_UART0_SCLK     = 4,

  PINMUX_UART1_TXD      = 5,
  PINMUX_UART1_RXD      = 6,
  PINMUX_UART1_RTS      = 7,
  PINMUX_UART1_CTS      = 8,
  PINMUX_UART1_SCLK     = 9,

  PINMUX_UART2_TXD      = 10,
  PINMUX_UART2_RXD      = 11,
  PINMUX_UART2_RTS      = 12,
  PINMUX_UART2_CTS      = 13,
  PINMUX_UART2_SCLK     = 14,

  PINMUX_SPI0_SCK       = 15,
  PINMUX_SPI0_MOSI      = 16,
  PINMUX_SPI0_MISO      = 17,
  PINMUX_SPI0_SSEL0     = 18,
  PINMUX_SPI0_SSEL1     = 19,
  PINMUX_SPI0_SSEL2     = 20,
  PINMUX_SPI0_SSEL3     = 21,

  PINMUX_SPI1_SCK       = 22,
  PINMUX_SPI1_MOSI      = 23,
  PINMUX_SPI1_MISO      = 24,
  PINMUX_SPI1_SSEL0     = 25,
  PINMUX_SPI1_SSEL1     = 26,

  PINMUX_SCT_PIN0       = 27,
  PINMUX_SCT_PIN1       = 28,
  PINMUX_SCT_PIN2       = 29,
  PINMUX_SCT_PIN3       = 30,
  PINMUX_SCT_OUT0       = 31,
  PINMUX_SCT_OUT1       = 32,
  PINMUX_SCT_OUT2       = 33,
  PINMUX_SCT_OUT3       = 34,
  PINMUX_SCT_OUT4       = 35,
  PINMUX_SCT_OUT5       = 36,

  PINMUX_I2C1_SDA       = 37,
  PINMUX_I2C1_SCL       = 38,

  PINMUX_I2C2_SDA       = 39,
  PINMUX_I2C2_SCL       = 40,

  PINMUX_I2C3_SDA       = 41,
  PINMUX_I2C3_SCL       = 42,

  PINMUX_ADC_PINTRIG0   = 43,
  PINMUX_ADC_PINTRIG1   = 44,
  PINMUX_ACMP_O         = 45,
  PINMUX_CLKOUT         = 46,
  PINMUX_GPIO_INT_BMAT  = 47,

  PINMUX_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_PIN_MUX_H_ */
