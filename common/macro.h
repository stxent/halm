/*
 * macro.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MACRO_H_
#define MACRO_H_
/*----------------------------------------------------------------------------*/
#define BIT(offset)                     ((uint32_t)1 << (offset))
#define BIT_FIELD(value, offset)        ((uint32_t)value << (offset))
/*----------------------------------------------------------------------------*/
#endif /* MACRO_H_ */
