/*
 * system.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm/stm32f1xx/system.h>
#include <halm/platform/stm/stm32f1xx/system_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  INDEX_AHB,
  INDEX_APB1,
  INDEX_APB2
};
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum SysClockBranch branch)
{
  const unsigned int index = branch >> 5;
  const uint32_t value = 1UL << (branch & 0x1F);

  switch (index)
  {
    case INDEX_AHB:
      STM_RCC->AHBENR |= value;
      break;

    case INDEX_APB1:
      STM_RCC->APB1ENR |= value;
      break;

    case INDEX_APB2:
      STM_RCC->APB2ENR |= value;
      break;
  }
}
/*----------------------------------------------------------------------------*/
void sysClockDisable(enum SysClockBranch branch)
{
  const unsigned int index = branch >> 5;
  const uint32_t mask = ~(1UL << (branch & 0x1F));

  switch (index)
  {
    case INDEX_AHB:
      STM_RCC->AHBENR &= mask;
      break;

    case INDEX_APB1:
      STM_RCC->APB1ENR &= mask;
      break;

    case INDEX_APB2:
      STM_RCC->APB2ENR &= mask;
      break;
  }
}
/*----------------------------------------------------------------------------*/
bool sysClockStatus(enum SysClockBranch branch)
{
  const unsigned int index = branch >> 5;
  const uint32_t mask = 1UL << (branch & 0x1F);
  uint32_t value = 0;

  switch (index)
  {
    case INDEX_AHB:
      value = STM_RCC->AHBENR;
      break;

    case INDEX_APB1:
      value = STM_RCC->APB1ENR;
      break;

    case INDEX_APB2:
      value = STM_RCC->APB2ENR;
      break;
  }

  return (value & mask) != 0;
}
/*----------------------------------------------------------------------------*/
unsigned int sysFlashLatency(void)
{
  return FLASH_ACR_LATENCY(STM_FLASH->ACR) + 1;
}
/*----------------------------------------------------------------------------*/
/**
 * Set the flash access time.
 * @param value Flash access time in CPU clocks.
 * @n Possible values and recommended operating frequencies:
 *   - 1 clock: up to 24 MHz.
 *   - 2 clocks: up to 48 MHz.
 *   - 3 clocks: up to 72 MHz.
 */
void sysFlashLatencyUpdate(unsigned int value)
{
  STM_FLASH->ACR = (STM_FLASH->ACR & ~FLASH_ACR_LATENCY_MASK)
      | FLASH_ACR_LATENCY(value - 1);
}
/*----------------------------------------------------------------------------*/
void sysResetEnable(enum SysBlockReset block)
{
  const unsigned int index = block >> 5;
  const uint32_t value = 1UL << (block & 0x1F);

  switch (index)
  {
    case INDEX_AHB:
      STM_RCC->AHBRSTR |= value;
      break;

    case INDEX_APB1:
      STM_RCC->APB1RSTR |= value;
      break;

    case INDEX_APB2:
      STM_RCC->APB2RSTR |= value;
      break;
  }
}
/*----------------------------------------------------------------------------*/
void sysResetDisable(enum SysBlockReset block)
{
  const unsigned int index = block >> 5;
  const uint32_t mask = ~(1UL << (block & 0x1F));

  switch (index)
  {
    case INDEX_AHB:
      STM_RCC->AHBRSTR &= mask;
      break;

    case INDEX_APB1:
      STM_RCC->APB1RSTR &= mask;
      break;

    case INDEX_APB2:
      STM_RCC->APB2RSTR &= mask;
      break;
  }
}
