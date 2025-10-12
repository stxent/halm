/*
 * halm/platform/lpc/lpc82x/sct_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SCT_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_SCT_BASE_H_
#define HALM_PLATFORM_LPC_LPC82X_SCT_BASE_H_
/*----------------------------------------------------------------------------*/
#define SCT_EVENT_COUNT 8

enum [[gnu::packed]] SctInput
{
  SCT_INPUT_NONE,
  SCT_INPUT_0,
  SCT_INPUT_1,
  SCT_INPUT_2,
  SCT_INPUT_3,
  SCT_INPUT_END
};

enum [[gnu::packed]] SctOutput
{
  SCT_OUTPUT_NONE,
  SCT_OUTPUT_0,
  SCT_OUTPUT_1,
  SCT_OUTPUT_2,
  SCT_OUTPUT_3,
  SCT_OUTPUT_4,
  SCT_OUTPUT_5,
  SCT_OUTPUT_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_SCT_BASE_H_ */
