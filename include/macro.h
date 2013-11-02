/*
 * macro.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef MACRO_H_
#define MACRO_H_
/*----------------------------------------------------------------------------*/
#define BIT(offset)                     (1UL << (offset))
#define BIT_FIELD(value, offset)        ((unsigned long)(value) << (offset))
/*----------------------------------------------------------------------------*/
#endif /* MACRO_H_ */
