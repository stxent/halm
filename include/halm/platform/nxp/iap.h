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
enum Result eepromReadBuffer(uint32_t, void *, size_t);
enum Result eepromWriteBuffer(uint32_t, const void *, size_t);
enum Result flashBlankCheckSector(uint32_t);
enum Result flashErasePage(uint32_t);
enum Result flashEraseSector(uint32_t);
void flashInitWrite(void);
uint32_t flashReadId(void);
uint32_t flashReadConfigId(void);
enum Result flashWriteBuffer(uint32_t, const void *, size_t);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_IAP_H_ */
