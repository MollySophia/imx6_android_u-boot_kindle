/*
 * Copyright (C) 2010-2012 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <common.h>
#include "registers.h"
#include "iomux_register.h"

typedef volatile unsigned long WORD;
typedef volatile unsigned short HALF;
typedef volatile unsigned char BYTE;

#define reg8_read(addr)       *((BYTE *)(addr))
#define reg8_write(addr,val)  *((BYTE *)(addr)) = (val)

#define reg16_read(addr)      *((HALF *)(addr))
#define reg16_write(addr,val) *((HALF *)(addr)) = (val)

#define reg32_read(addr)      *((WORD *)(addr))
#define reg32_write(addr,val) *((WORD *)(addr)) = (val)

#define  reg32setbit(addr,bitpos) \
         reg32_write((addr),(reg32_read((addr)) | ((WORD) 1<<(bitpos))))

#endif  /* COMMON_H_ */
