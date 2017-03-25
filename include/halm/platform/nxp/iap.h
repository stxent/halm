/*
 * halm/platform/nxp/iap.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_IAP_H_
#define HALM_PLATFORM_NXP_IAP_H_
/*----------------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
#include <xcore/error.h>
/*----------------------------------------------------------------------------*/
enum result eepromReadBuffer(uint32_t, void *, size_t);
enum result eepromWriteBuffer(uint32_t, const void *, size_t);
enum result flashBlankCheckSector(uint32_t);
enum result flashErasePage(uint32_t);
enum result flashEraseSector(uint32_t);
void flashInitWrite(void);
uint32_t flashReadId(void);
uint32_t flashReadConfigId(void);
enum result flashWriteBuffer(uint32_t, const void *, size_t);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_IAP_H_ */
