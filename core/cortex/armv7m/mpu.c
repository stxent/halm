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
/**
 * Compute MPU region parameters with given attributes.
 *
 * Calculates the optimal region configuration (base address and size)
 * for the MPU, ensuring proper alignment and power‑of‑two size requirements.
 * Handles subregion disable mask calculation for large regions (> 128 bytes).
 *
 * @param address Starting address of the memory region to protect. Will be
 * aligned down to the nearest region boundary during computation.
 * @param size Requested size of the region in bytes. Must be non‑zero.
 * The function rounds this up to the next power of two if necessary.
 * @param attributes Pre‑computed MPU attribute bits.
 * @param config Pointer to a structure where the computed configuration
 * will be stored.
 * @return @b true if the region configuration was computed successfully
 * or @b false if the configuration cannot be computed.
 */
static bool computeAttributedRegion(uintptr_t address, size_t size,
    uint32_t attributes, struct MpuRegionConfig *config)
{
  uint32_t regionSizePow = 31 - countLeadingZeros32((uint32_t)size);

  if (size & ((1UL << regionSizePow) - 1))
  {
    /* Size is not a power of two */
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
    /*
     * Fill Subregion Disable bits when the region size is greater
     * than 128 bytes.
     */

    const uint32_t subSizePow = regionSizePow - 3;
    const uint32_t subMask = (1UL << subSizePow) - 1;
    unsigned int sub;

    /* Disable subregions at the beginning of the region */
    sub = (address - regionBegAddress) >> subSizePow;
    subregions |= (1 << sub) - 1;

    /* Disable subregions at the end of the region */
    sub = (address + size - regionBegAddress + subMask) >> subSizePow;
    subregions |= 0xFF ^ ((1 << sub) - 1);
  }

  config->address = regionBegAddress;
  config->control = RASR_ENABLE | RASR_SIZE(regionSizePow - 1)
      | RASR_SRD(subregions) | attributes;

  return true;
}
/*----------------------------------------------------------------------------*/
/**
 * Find a free (unused) region in the Memory Protection Unit.
 *
 * Scans all available MPU regions and returns the index of the first region
 * that is currently disabled. A region is considered free if both
 * the enable bit and the size field are zero in its RASR register.
 *
 * @return Index of the first free MPU region if found or -1 if no free regions
 * are available.
 */
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
 * Add a new region to the Memory Protection Unit.
 *
 * This function computes the MPU configuration and assigns it to a free region.
 * If successful, the region is configured and enabled in the MPU.
 *
 * @param address Starting address of the region, must be aligned appropriately.
 * @param size Region size in bytes, must be a power of two, minimum 32 bytes.
 * @param preset MPU configuration preset defining memory type
 * and caching attributes.
 * @param access Privileged and unprivileged access permissions for the region.
 * @param executable If @b true, enables instruction fetches from the region;
 * if @b false, disables them.
 * @param shareable If @b true, marks the region as shared.
 * @return Region identifier on success, or -1 if no free region is available
 * or configuration fails.
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
      /* RNR register is already initialized by findFreeRegion() */
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
 * Compute an MPU region configuration based on input parameters.
 *
 * Calculates the control register bits (RASR) and base address (RBAR)
 * for a given region. Handles alignment, size rounding, and attribute mapping.
 *
 * @param address Starting address of the region. Will be aligned down
 * to the nearest region boundary.
 * @param size Requested region size in bytes. Must be at least 32 bytes;
 * will be rounded up to the next power of two if needed.
 * @param preset Memory type preset.
 * @param access Access permissions for privileged and unprivileged modes.
 * @param executable If @b false, sets the Execute Never bit to prevent
 * instruction fetches.
 * @param shareable If @b true and preset is not DEVICE, sets the Shareable bit.
 * @param config Pointer to an MPU region configuration structure to be filled.
 * @return @b true if the configuration was computed successfully,
 * @b false otherwise.
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
/**
 * Reconfigure an existing MPU region with new settings.
 *
 * Updates the address and control registers of a specified MPU region.
 * The region must already be allocated.
 *
 * @param region Identifier of the region to reconfigure.
 * @param config Pointer to the new MPU region configuration.
 */
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
/**
 * Remove and disable an MPU region.
 *
 * Clears the control and base address registers for the specified region,
 * effectively disabling it.
 *
 * @param region Identifier of the region to remove.
 */
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
/**
 * Reserve an MPU region without configuring it.
 *
 * Allocates a free region and marks it as reserved by setting a special
 * size value. This prevents other functions from using the region until
 * it is explicitly configured.
 *
 * @return Reserved region identifier on success, or -1 if no free region
 * is available.
 */
MpuRegion mpuReserveRegion(void)
{
  const MpuRegion region = findFreeRegion();

  if (region != -1)
  {
    /* RNR register is already initialized by findFreeRegion() */
    MPU->RASR = RASR_SIZE_RESERVED;

    __dsb();
    __isb();
  }

  return region;
}
/*----------------------------------------------------------------------------*/
/**
 * Disable the Memory Protection Unit.
 *
 * Turns off the MPU, removing all memory protection.
 */
void mpuDisable(void)
{
  __dsb();

  MPU->CTRL = 0;

  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
/**
 * Enable the Memory Protection Unit.
 *
 * Activates the MPU with default settings:
 *   - Enables the unit.
 *   - Allows default memory map for privileged mode.
 */
void mpuEnable(void)
{
  __dsb();

  MPU->CTRL = CTRL_ENABLE | CTRL_PRIVDEFENA;

  __dsb();
  __isb();
}
/*----------------------------------------------------------------------------*/
/**
 * Reset the Memory Protection Unit to its default state.
 *
 * Performs a full reset of the MPU:
 *   - Disables the unit.
 *   - Clears all region configurations.
 */
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
