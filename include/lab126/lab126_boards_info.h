/*
 * lab126_boards_info.h
 *
 * Copyright 2010-2015 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#include <boardid.h>

#define MEMORY_TYPE_UNKNOWN	0
#define MEMORY_TYPE_MDDR	1
#define MEMORY_TYPE_LPDDR2	2

#define MEMORY_SIZE_256MB       (256 * 1024 * 1024)
#define MEMORY_SIZE_512MB       (512 * 1024 * 1024)
#define MEMORY_SIZE_1GB         (1024 * 1024 * 1024)

typedef struct board_type {
    const char *id;  /* Tattoo + rev */
    const char *name;
    unsigned char mem_type;
    unsigned int mem_size;
} board_type;

static const struct board_type boards[] = {
    {
	.id = BOARD_ID_HEISENBERG,
	.name = "Heisenberg",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_EANAB_WFO,
	.name = "Eanab",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    },
    {
	.id = BOARD_ID_EANAB_BAIJIU,
	.name = "Baijiu",
	.mem_type = MEMORY_TYPE_LPDDR2,
	.mem_size = MEMORY_SIZE_512MB,
    }
};

#define NUM_KNOWN_BOARDS  (sizeof(boards)/sizeof(boards[0]))


