/*
 * macro.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MACRO_H_
#define MACRO_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define BIT(offset)                     ((uint32_t)1 << (offset))
#define BIT_FIELD(value, offset)        ((uint32_t)value << (offset))
/*----------------------------------------------------------------------------*/
/* TODO Move definition to other header */
extern inline void usleep(uint32_t period);
extern inline void msleep(uint32_t period);
/*----------------------------------------------------------------------------*/
#endif /* MACRO_H_ */
