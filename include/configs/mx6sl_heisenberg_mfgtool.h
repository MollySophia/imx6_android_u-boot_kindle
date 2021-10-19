/*
 * Copyright (C) 2013-2014 Freescale Semiconductor, Inc.
 * Copyright (C) 2015 Amazon.com, Inc.
 *
 * Configuration settings for the Freescale i.MX6SL EVK board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_DCACHE_OFF

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/imx-common/gpio.h>

#define CONFIG_EANAB

#define CONFIG_MX6
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* uncomment for PLUGIN mode support */
/* #define CONFIG_USE_PLUGIN */

/* uncomment for SECURE mode support */
/* #define CONFIG_SECURE_BOOT */

#define MACH_TYPE_MX6SLEVK		4307
#define CONFIG_MACH_TYPE		MACH_TYPE_MX6SLEVK

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(3 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MXC_GPIO

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_IPS_BASE_ADDR

/* OCOTP Config */
#define CONFIG_CMD_FUSE
#ifdef CONFIG_CMD_FUSE
#define CONFIG_MXC_OCOTP
#endif

#define CONFIG_IMXOTP
#define IMX_OTP_BASE			OCOTP_BASE_ADDR
#define IMX_OTP_ADDR_MAX	0x7F
#define IMX_OTP_DATA_ERROR_VAL	0xBADABADA


/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM 3

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
//#define CONFIG_CMD_EXT2
#define CONFIG_EFI_PARTITION

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */
#define CONFIG_SYS_EMMC_BOOT_PARTITION	2
#define CONFIG_MMC_DEFAULT_DEV		1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* I2C configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		375000
#define CONFIG_SYS_MXC_I2C1_SPEED	375000
#define CONFIG_SYS_MXC_I2C2_SPEED	375000
#define CONFIG_SYS_MXC_I2C3_SPEED	375000

/* PMIC config */
/*
#define CONFIG_PMIC_MAX77696
#define CONFIG_PMIC_I2C
#define CONFIG_MX6_INTER_LDO_BYPASS	1
#define CONFIG_MAX77696_FG_INIT		1
*/
#define CONFIG_PMIC_ROHM
#define CONFIG_PMIC_I2C
#define CONFIG_PMIC_FITIPOWER
/**************************************************************
 * Definition for IDME
***************************************************************/

#define CONFIG_CMD_IDME         1

#define CONFIG_IDME_UPDATE              1
//#define CONFIG_IDME_UPDATE_ADDR         0x3f000
#define CONFIG_IDME_UPDATE_MAGIC        "abcdefghhgfedcba"
#define CONFIG_IDME_UPDATE_MAGIC_SIZE   16


/* Command definition */
#include <config_cmd_default.h>

#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY		3
#define CONFIG_LOADADDR			0x80800000
#define CONFIG_SYS_TEXT_BASE		0x81000000

#define CONFIG_MFG_ENV_SETTINGS \
	"mfgtool_args=setenv bootargs console=${console},${baudrate} " \
		"rdinit=/linuxrc " \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 " \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" "\
		"\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffff\0" \
	"bootcmd_mfg=run mfgtool_args;bootm ${loadaddr} ${initrd_addr} ${fdt_addr};\0" \
	"fdt_addr=0x83000000\0" \

/* the Image has device tree and uImage concatenated together
 * It has dtb padded to 100K which follows by uImage
 * uboot to read the dtb from the disk 
 * at the beginning of the Image - block 0x208
 * read uImage at 100K offset from the Image 0x2D0=0x208+0xC8 
 * (1024*100/512=0xC8)
 */  
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=uImage\0" \
	"console=ttymxc0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx6sl-abc123.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"boot_fdt=no\0" \
	"mmcdev="__stringify(CONFIG_MMC_DEFAULT_DEV)"\0" \
	"mmcpart="__stringify(CONFIG_SYS_EMMC_BOOT_PARTITION)"\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=ext2load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=ext2load mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"echo Loading image from hardcoded offset....; " \
			"mmc read ${loadaddr} 208 3000; " \
			"bootm ${loadaddr}; " \
		"fi;\0" \
	"setmmcdev=mmc dev 1 0; \0"\
	"loadkernel=mmc read 0x80800000 0x2D0 0x7000; \0" \
	"loaddt=mmc read 0x83000000 0x208 0x100; \0" \
	"testboot=echo test BOOTing from mmc ...; "\
		"run setmmcdev; " \
		"run loadkernel; " \
		"run loaddt; " \
		"run mmcargs; "\
		"bootm ${loadaddr} - ${fdt_addr} " \


/*
#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev};" \
	"if mmc rescan; then " \
			"run mmcboot; " \
	"fi; " \
*/

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev};" \
	"if mmc rescan; then " \
			"run testboot; " \
	"fi; " \

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		1024

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		256
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
//
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x84000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + SZ_512K)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE		SZ_128K

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_512M

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE			SZ_8K
#define CONFIG_SYS_MMC_ENV_DEV		1   /* USDHC2 */
#define CONFIG_SYS_MMC_ENV_PART		0	/* user partition */
#define CONFIG_MMCROOT			"/dev/mmcblk1p1"  /* USDHC2 */

#if defined CONFIG_SYS_BOOT_SPINOR
#define CONFIG_SYS_USE_SPINOR
#define CONFIG_ENV_IS_IN_SPI_FLASH
#else
#define CONFIG_ENV_IS_IN_MMC
#endif

#ifdef CONFIG_SYS_USE_SPINOR
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS  0
#define CONFIG_SF_DEFAULT_SPEED 20000000
#define CONFIG_SF_DEFAULT_MODE (SPI_MODE_0)
#define CONFIG_SF_DEFAULT_CS   (0|(IMX_GPIO_NR(4, 11)<<8))
#endif

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(8 * 64 * 1024)
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#endif

#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

/* LP5523 LED Control */
/*
#define CONFIG_LP5523_LED_CONTROL		1
#define CONFIG_LP5523_NUM_DRIVERS		1
#define CONFIG_LP5523_I2C_BUS		1
*/

/*#define CONFIG_SPLASH_SCREEN*/

/*
 * SPLASH SCREEN Configs
 */
#ifdef CONFIG_SPLASH_SCREEN
	/*
	 * Framebuffer and LCD
	 */
	#define CONFIG_CMD_BMP
	#define CONFIG_MXC_EPDC				1
	#define CONFIG_LCD
	#define CONFIG_FB_BASE				(CONFIG_SYS_TEXT_BASE + 0x300000)
	#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#ifdef CONFIG_MXC_EPDC
	#undef LCD_TEST_PATTERN
	/* #define CONFIG_SPLASH_IS_IN_MMC			1 */
	#define LCD_BPP					LCD_MONOCHROME
	/* #define CONFIG_SPLASH_SCREEN_ALIGN		1 */

	#define CONFIG_WORKING_BUF_ADDR			(CONFIG_SYS_TEXT_BASE + 0x100000)
	#define CONFIG_WAVEFORM_BUF_ADDR		(CONFIG_SYS_TEXT_BASE + 0x200000)
	#define CONFIG_WAVEFORM_FILE_OFFSET		0x600000
	#define CONFIG_WAVEFORM_FILE_SIZE		0xF0A00
	#define CONFIG_WAVEFORM_FILE_IN_MMC

#ifdef CONFIG_SPLASH_IS_IN_MMC
	#define CONFIG_SPLASH_IMG_OFFSET		0x4c000
	#define CONFIG_SPLASH_IMG_SIZE			0x19000
#endif
#endif
#endif /* CONFIG_SPLASH_SCREEN */


//#define CONFIG_SYS_LDSCRIPT      "board/$(BOARDDIR)/u-boot-diag.lds"

#ifdef CONFIG_SYS_PROMPT
#undef CONFIG_SYS_PROMPT
#endif
#define CONFIG_SYS_PROMPT "mfgtool > "

#define CONFIG_MFGTOOL 1

//#define CONFIG_PMIC
//#define CONFIG_CMD_PMIC
//#define CONFIG_CMD_INVENTORY
//#define CONFIG_CMD_VNI
//#define CONFIG_CMD_LPM
//#define CONFIG_FSL_MX6SL_LPM_TEST
//#define CONFIG_FSL_MX6SL_LPM_VMA	0x920000

//#define CONFIG_LAB126_LPM_TEST
//#define CONFIG_CMD_DIAG

//#define CONFIG_WIFI_CMD
#define CONFIG_CMD_CRC
#define CRC32_BUFFER        0x02000000 + PHYS_SDRAM
#define CRC32_BUFFER_SIZE       0x00400000
#define CRC32_CHECK_UBOOT       0x00000010      /* check crc32 on u-boot.bin */
#define CRC32_CHECK_UIMAGE      0x00000020      /* check crc32 on uImage */
#define CRC32_CHECK_ROOTFS      0x00000040      /* check crc32 on rootfs.img */
#define CRC32_CHECK_MBR         0x00000080      /* check crc32 on mbr-xxx-bin */

#include <lab126/lab126_partitions_def.h>
#include "mx6sl_abc123_android.h"
#undef CONFIG_FASTBOOT_PRODUCT_NAME_STR
/* Override some #define's from the above include */
#define CONFIG_FASTBOOT_PRODUCT_NAME_STR		"i.mx6sl eanab"

/* store aboot mode flag in either PMIC or SOC SNVC */
#define CONFIG_ABOOT_FLAG_PMIC
//#define CONFIG_ABOOT_FLAG_SNVS

#endif				/* __CONFIG_H */
