/*
 * lab126_partitions_info.h
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

#ifndef __MX60_EANAB_BOARD_H__
#define __MX60_EANAB_BOARD_H__

#define PARTITION_FILL_SPACE	-1


typedef struct partition_info_t {
    const char *name;
    unsigned int address;
    unsigned int size;
    unsigned int partition;
} partition_info_t;


const struct partition_info_t partition_info_default[CONFIG_NUM_PARTITIONS] = {
	{
		.name = "bootloader",
		.address = 0,
		.size = CONFIG_MMC_USERDATA_ADDR, //512k
		.partition = CONFIG_BOOT_FROM_PARTITION
	},
	{
		.name = "spl",
		.address = 0x0, /* overlap with bootloader */
		.size = CONFIG_MMC_SPL_PARTITION_SIZE, //512k
		.partition = CONFIG_MMC_SPL_PARTITION //1
	},
	{
		.name = "mboot",
		.address = CONFIG_MMC_MBOOT_ADDR,
		.size = CONFIG_MMC_MBOOT_PARTITION_SIZE, //768k
		.partition = CONFIG_MMC_MBOOT_PARTITION //2
	},
	{
		.name = "idme_data",
		.address = CONFIG_MMC_USERDATA_ADDR,
		.size = CONFIG_MMC_USERDATA_SIZE,  /* 5 KiB */
		.partition = CONFIG_BOOT_FROM_PARTITION
	},
	{
		.name = "gpt",
		.address = 0,
		.size = 17 * 1024,  /* 17 KiB */
		.partition = 0
	}
};

#endif /* __MX60_EANAB_BOARD_H__ */
