/*
 * halm/platform/linux/mmf.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LINUX_MMF_H_
#define HALM_PLATFORM_LINUX_MMF_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const MemoryMappedFile;
/*----------------------------------------------------------------------------*/
struct MbrDescriptor
{
  uint32_t offset;
  uint32_t size;
  uint8_t type;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result mmfSetPartition(void *, struct MbrDescriptor *);
enum Result mmfReadTable(void *, uint32_t, uint8_t, struct MbrDescriptor *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LINUX_MMF_H_ */
