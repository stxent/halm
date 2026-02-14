/*
 * halm/platform/numicro/m03x/bod.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_BOD_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_BOD_H_
#define HALM_PLATFORM_NUMICRO_M03X_BOD_H_
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] BodLevel
{
  BOD_LEVEL_2V0,
  BOD_LEVEL_2V5,

  BOD_LEVEL_END
};

enum [[gnu::packed]] BodTimeout
{
  BOD_TIMEOUT_LSC_DIV4,
  BOD_TIMEOUT_HCLK_64,
  BOD_TIMEOUT_HCLK_128,
  BOD_TIMEOUT_HCLK_256,
  BOD_TIMEOUT_HCLK_512,
  BOD_TIMEOUT_HCLK_1024,
  BOD_TIMEOUT_HCLK_2048,
  BOD_TIMEOUT_HCLK_4096,

  BOD_TIMEOUT_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_BOD_H_ */
