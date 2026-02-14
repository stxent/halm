/*
 * halm/platform/numicro/m48x/bod.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_BOD_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_BOD_H_
#define HALM_PLATFORM_NUMICRO_M48X_BOD_H_
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] BodLevel
{
  BOD_LEVEL_1V6,
  BOD_LEVEL_1V8,
  BOD_LEVEL_2V0,
  BOD_LEVEL_2V2,
  BOD_LEVEL_2V4,
  BOD_LEVEL_2V6,
  BOD_LEVEL_2V8,
  BOD_LEVEL_3V0,

  BOD_LEVEL_END
};

enum [[gnu::packed]] BodTimeout
{
  BOD_TIMEOUT_LSC,
  BOD_TIMEOUT_HCLK_4,
  BOD_TIMEOUT_HCLK_8,
  BOD_TIMEOUT_HCLK_16,
  BOD_TIMEOUT_HCLK_32,
  BOD_TIMEOUT_HCLK_64,
  BOD_TIMEOUT_HCLK_128,
  BOD_TIMEOUT_HCLK_256,

  BOD_TIMEOUT_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_BOD_H_ */
