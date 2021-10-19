/*
 * lab126_partitions.h
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
#define CONFIG_BOOT_PARTITION_ACCESS
#define CONFIG_BOOT_FROM_PARTITION      1
#define CONFIG_BOOT_FROM_PARTITION_2    2
#define CONFIG_NUM_PARTITIONS           6

#define CONFIG_MMC_BOOTFLASH            1
#define CONFIG_MMC_USERDATA_ADDR        0x80000 // 512 K
#define CONFIG_MMC_USERDATA_SIZE        (5*1024)
#define CONFIG_MMC_SPL_PARTITION_SIZE   CONFIG_MMC_USERDATA_ADDR
#define CONFIG_MMC_SPL_PARTITION        CONFIG_BOOT_FROM_PARTITION
#define CONFIG_MMC_MBOOT_ADDR           0x0
#define CONFIG_MMC_MBOOT_SIGNED_SIZE    0x80000 // 512 K
#define CONFIG_MMC_MBOOT_PARTITION_SIZE 0xC0000 // 768 K
#define CONFIG_MMC_MBOOT_PARTITION      CONFIG_BOOT_FROM_PARTITION_2
#define CONFIG_MMC_MAX_TRANSFER_SIZE    (0xFFFF * 512)

#define CONFIG_IDME_UPDATE_ADDR         0x3F000
