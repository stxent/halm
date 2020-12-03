/*
 * halm/core/cortex/armv7em/fpu.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_ARMV7EM_FPU_H_
#define HALM_CORE_CORTEX_ARMV7EM_FPU_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void fpuDisable(void);
void fpuEnable(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7EM_FPU_H_ */
