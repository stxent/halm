/*
 * bits.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef BITS_H_
#define BITS_H_
/*----------------------------------------------------------------------------*/
#define BIT(shift)                      (1UL << (shift))
#define BIT_FIELD(value, shift)         ((unsigned long)(value) << (shift))
#define FIELD_VALUE(reg, mask, shift)   (((reg) & (mask)) >> (shift))
#define MASK(width)                     ((1UL << (width)) - 1)
/*----------------------------------------------------------------------------*/
#endif /* BITS_H_ */
