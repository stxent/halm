/*
 * mpu.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/core_defs.h>
#include <halm/core/cortex/armv7m/mpu_defs.h>
#include <halm/core/cortex/mpu.h>
#include <xcore/accel.h>
#include <xcore/asm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool computeAttributedRegion(uintptr_t, size_t, uint32_t,
    struct MpuRegionConfig *);
static MpuRegion findFreeRegion(void);
/*----------------------------------------------------------------------------*/
static bool computeAttributedRegion(uintptr_t address, size_t size,
    uint32_t attributes, struct MpuRegionConfig *config)
{
  uint32_t regionSizePow = 31 - countLeadingZeros32((uint32_t)size);

  if (size & ((1UL << regionSizePow) - 1))
  {
    /* Size is not power of two */
    ++regionSizePow;
  }

  if (regionSizePow < 5)
  {
    /* Region size is lower than 32 bytes */
    return false;
  }

  uint32_t regionBegAddress = address & ~((1UL << regionSizePow) - 1);

  if ((uint32_t)size + address - regionBegAddress > (1UL << regionSizePow))
  {
    /*
     * End of the input region is not aligned to the computed region size,
     * increase region size.
     */
    ++regionSizePow;
  }

  const uint32_t regionSize = 1UL << regionSizePow;
  const uint32_t regionMask = regionSize - 1;
  uint8_t subregions = 0;

  regionBegAddress = address & ~regionMask;

  if (regionSize > 128)
  {
    /* Fill Subregion Disable bits when a region size is greater than 128 */

    const uint32_t subSizePow = regionSizePow - 3;
    const uint32_t subMask = (1UL << subSizePow) - 1;
    unsigned int sub;

    /* Disable subregions in the beginning of the region */
    sub = (address - regionBegAddress) >> subSizePow;
    subregions |= (1 << sub) - 1;

    /* Disable subregions in the end of the region */
    sub = (address + size - regionBegAddress + subMask) >> subSizePow;
    subregions |= 0xFF ^ ((1 << sub) - 1);
  }

  config->address = regionBegAddress;
  config->control = RASR_ENABLE | RASR_SIZE(regionSizePow - 1)
      | RASR_SRD(subregions) | attributes;

  return true;
}
/*----------------------------------------------------------------------------*/
static MpuRegion findFreeRegion(void)
{
  const unsigned int count = TYPE_DREGION_VALUE(MPU->TYPE);

  __dsb();

  for (unsigned int index = 0; index < count; ++index)
  {
    MPU->RNR = index;

    if (!(MPU->RASR & (RASR_ENABLE | RASR_SIZE_MASK)))
      return index;
  }

  return -1;
}
/*----------------------------------------------------------------------------*/
/**
 * Add a new region to Memory Protection Unit.
 * @param address Starting address of the region.
 * @param size Region size in bytes.
 * @param preset MPU configuration preset for the region.
 * @param access Privileged and unprivileged access permissions.
 * @param executable Enable instruction fetches for the region.
 * @param shareabe Mark the region as shared, used in multiprocessor systems.
 * @return Region identifier on success or a negative number in case of failure.
 */
MpuRegion mpuAddRegion(uintptr_t address, size_t size, enum MpuPreset preset,
    enum MpuAccessPermission access, bool executable, bool shareable)
{
  struct MpuRegionConfig config;
  MpuRegion region = -1;

  if (mpuComputeRegion(address, size, preset, access, executable, shareable,
      &config))
  {
    if ((region = findFreeRegion()) != -1)
    {
      /* RNR register is already initialized */
      MPU->RBAR = config.address;
      MPU->RASR = config.control;

      __dsb();
      __isb();
    }
  }

  return region;
}
/*----------------------------------------------------------------------------*/
/**
 * Compute an MPU configuration.
 * @param address Starting address of the region.
 * @param size Region size in bytes.
 * @param preset MPU configuration preset for the region.
 * @param access Privileged and unprivileged access permissions.
 * @param executable Enable instruction fetches for the region.
 * @param shareabe Mark the region as shared, used in multiprocessor systems.
 * @param config Pointer to an MPU region configration structure.
 * @return Status of the operation.
 */
bool mpuComputeRegion(uintptr_t address, size_t size, enum MpuPreset preset,
    enum MpuAccessPermission access, bool executable, bool shareable,
    struct MpuRegionConfig *config)
{
  static const uint8_t accessPermissionMap[] = {
      [MPU_ACCESS_FULL] = AP_FULL_ACCESS,
      [MPU_ACCESS_NONE] = AP_NO_ACCESS,
      [MPU_ACCESS_READ_ONLY] = AP_READ_ONLY,
      [MPU_ACCESS_RW_NA] = AP_RW_NA,
      [MPU_ACCESS_RW_RO] = AP_RW_RO,
      [MPU_ACCESS_RO_NA] = AP_RO_NA
  };

  assert(access < ARRAY_SIZE(accessPermissionMap));

  uint32_t attributes = RASR_AP(accessPermissionMap[access]);

  switch (preset)
  {
    case MPU_REGION_DEVICE:
      attributes |= shareable ? RASR_B : RASR_TEX(2);
      break;

    case MPU_REGION_NORMAL_NONCACHEABLE:
      attributes |= RASR_TEX(1);
      break;

    case MPU_REGION_NORMAL_WRITE_BACK:
      attributes |= RASR_B | RASR_C;
      break;

    case MPU_REGION_NORMAL_WRITE_BACK_RW_ALLOCATE:
      attributes |= RASR_B | RASR_C | RASR_TEX(1);
      break;

    case MPU_REGION_NORMAL_WRITE_THROUGH:
      attributes |= RASR_C;
      break;

    default:
      break;
  }

  if (shareable && preset != MPU_REGION_DEVICE)
    attributes |= RASR_S;

  if (!executable)
    attributes |= RASR_XN;

  return computeAttributedRegion(address, size, attributes, config);
}
/*----------------------------------------------------------------------------*/
void mpuReconfigureRegion(MpuRegion region,
    const struct MpuRegionConfig *config)
{
  __dsb();

  MPU->RNR = region;
  MPU->RASR = 0;
  MPU->RBAR = config->address;
  MPU->RASR = config->control;

  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
void mpuRemoveRegion(MpuRegion region)
{
  assert(region >= 0 && region < (MpuRegion)TYPE_DREGION_VALUE(MPU->TYPE));

  __dsb();

  MPU->RNR = (uint32_t)region;
  MPU->RASR = 0;
  MPU->RBAR = 0;

  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
MpuRegion mpuReserveRegion(void)
{
  const MpuRegion region = findFreeRegion();

  if (region != -1)
  {
    /* RNR register is already initialized */
    MPU->RASR = RASR_SIZE_RESERVED;

    __dsb();
    __isb();
  }

  return region;
}
/*----------------------------------------------------------------------------*/
void mpuDisable(void)
{
  __dsb();

  MPU->CTRL = 0;

  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
void mpuEnable(void)
{
  __dsb();

  MPU->CTRL = CTRL_ENABLE | CTRL_PRIVDEFENA;

  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
void mpuReset(void)
{
  const unsigned int count = TYPE_DREGION_VALUE(MPU->TYPE);

  __dsb();

  MPU->CTRL = 0;

  for (unsigned int index = 0; index < count; ++index)
  {
    MPU->RNR = index;
    MPU->RASR = 0;
    MPU->RBAR = 0;
  }

  __dsb();
  __isb();
}
