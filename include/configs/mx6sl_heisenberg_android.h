
/*
 * Copyright (C) 2013-2014 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef MX6SL_HEISENBERG_ANDROID_H
#define MX6SL_HERSENBERG_ANDROID_H

#include <asm/imx-common/mxc_key_defs.h>

#define CONFIG_SERIAL_TAG

#define CONFIG_USB_DEVICE
#define CONFIG_IMX_UDC					1

#define CONFIG_FASTBOOT					1
#define CONFIG_FASTBOOT_VENDOR_ID			0x1949
#define CONFIG_FASTBOOT_PRODUCT_ID			0xd0d0
#define CONFIG_FASTBOOT_BCD_DEVICE			0x311
#define CONFIG_FASTBOOT_MANUFACTURER_STR		"Lab126"
#define CONFIG_FASTBOOT_PRODUCT_NAME_STR		"i.mx6sl Wario Board"
#define CONFIG_FASTBOOT_INTERFACE_STR			"Android fastboot"
#define CONFIG_FASTBOOT_CONFIGURATION_STR		"Android fastboot"
#define CONFIG_FASTBOOT_SERIAL_NUM			"123456789"
#define CONFIG_FASTBOOT_SATA_NO				0

#define CONFIG_FASTBOOT_STORAGE_MMC
//#define CONFIG_FASTBOOT_STORAGE_VENDOR_PTABLE

/*  For system.img growing up more than 256MB, more buffer needs
*   to receive the system.img*/
#define CONFIG_FASTBOOT_TRANSFER_BUF			0x83000000
#define CONFIG_FASTBOOT_TRANSFER_BUF_SIZE		0x1C800000 /* 456 Mbyte */

/* Android specific */
#if defined(CONFIG_ANDROID_SUPPORT)

#define CONFIG_CMD_BOOTI
#define CONFIG_ANDROID_RECOVERY
/* which mmc bus is your main storage ? */
#define CONFIG_ANDROID_MAIN_MMC_BUS				1
#define CONFIG_ANDROID_BOOT_PARTITION_MMC		1
#define CONFIG_ANDROID_SYSTEM_PARTITION_MMC		5
#define CONFIG_ANDROID_RECOVERY_PARTITION_MMC	2
#define CONFIG_ANDROID_CACHE_PARTITION_MMC		6

/*keyboard mapping*/
#define CONFIG_VOL_DOWN_KEY     KEY_BACK
#define CONFIG_POWER_KEY        KEY_5

#define CONFIG_MXC_KPD
#define CONFIG_MXC_KEYMAPPING \
	{       \
		KEY_SELECT, KEY_BACK, KEY_1,     KEY_2, \
		KEY_3,      KEY_4,    KEY_5,     KEY_MENU, \
		KEY_6,      KEY_7,    KEY_8,     KEY_9, \
		KEY_UP,     KEY_LEFT, KEY_RIGHT, KEY_DOWN, \
	}
#define CONFIG_MXC_KPD_COLMAX 4
#define CONFIG_MXC_KPD_ROWMAX 4


#undef CONFIG_EXTRA_ENV_SETTINGS
//#undef CONFIG_BOOTCOMMAND

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_EANAB_EXTRA_ENV_SETTINGS				\
	"splashpos=m,m\0"	  \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \

#endif

#endif /* CONFIG_ANDROID_SUPPORT */
