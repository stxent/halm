/*
 * halm/core/cortex/armv7m/mpu.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_CORE_CORTEX_MPU_H_
#error This header should not be included directly
#endif

#ifndef HALM_CORE_CORTEX_ARMV7M_MPU_H_
#define HALM_CORE_CORTEX_ARMV7M_MPU_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
#include <xcore/helpers.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
typedef int8_t MpuRegion;

struct MpuRegionConfig
{
  uint32_t address;
  uint32_t control;
};

enum [[gnu::packed]] MpuAccessPermission
{
  MPU_ACCESS_FULL,
  MPU_ACCESS_NONE,
  MPU_ACCESS_READ_ONLY,
  MPU_ACCESS_RW_NA,
  MPU_ACCESS_RW_RO,
  MPU_ACCESS_RO_NA
};

enum [[gnu::packed]] MpuPreset
{
  MPU_REGION_STRONGLY_ORDERED,
  MPU_REGION_DEVICE,
  MPU_REGION_NORMAL_NONCACHEABLE,
  MPU_REGION_NORMAL_WRITE_BACK,
  MPU_REGION_NORMAL_WRITE_BACK_RW_ALLOCATE,
  MPU_REGION_NORMAL_WRITE_THROUGH
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

MpuRegion mpuAddRegion(uintptr_t, size_t, enum MpuPreset,
    enum MpuAccessPermission, bool, bool);
bool mpuComputeRegion(uintptr_t, size_t, enum MpuPreset,
    enum MpuAccessPermission, bool, bool, struct MpuRegionConfig *);
void mpuReconfigureRegion(MpuRegion, const struct MpuRegionConfig *);
void mpuRemoveRegion(MpuRegion);
MpuRegion mpuReserveRegion(void);
void mpuDisable(void);
void mpuEnable(void);

void mpuReset(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CORE_CORTEX_ARMV7M_MPU_H_ */
