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

// This file contains all of the SoC Independant functions for the Common API.

#include "common.h"

WORD word_field_wr(WORD cur_word, WORD field, WORD start, WORD length)
    __attribute__ ((section (".low_power_code")));
WORD word_field_wr(WORD cur_word, WORD field, WORD start, WORD length){
    WORD mask = (((1 << length) - 1) << start);
    return ((WORD)(cur_word & (~mask)) | ((field << start) & mask));
}

WORD word_field_rd(WORD cur_word, WORD start, WORD length)
    __attribute__ ((section (".low_power_code")));
WORD word_field_rd(WORD cur_word, WORD start, WORD length){
    return ((WORD)(cur_word >> start) & ((1 << length) - 1));
}
