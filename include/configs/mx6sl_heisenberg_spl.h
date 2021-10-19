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

#define CONFIG_SECURE_BOOT // HAB Secure Boot
 
#define AMZ_SPL_BUILD
#undef CONFIG_CMD_BOOTM
#undef CONFIG_BOOTM_LINUX
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS
#undef CONFIG_BOOTM_VXWORKS

#undef CONFIG_CMD_BOOTM
#undef CONFIG_CMD_CRC32
#undef CONFIG_CMD_EXPORTENV
#undef CONFIG_CMD_GO
#undef CONFIG_CMD_IMPORTENV

#define CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_DCACHE_OFF

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/imx-common/gpio.h>

#define CONFIG_EANAB

#define CONFIG_IRAM_BOOT

#define CONFIG_MX6
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define MACH_TYPE_MX6SLEVK		4307
#define CONFIG_MACH_TYPE		MACH_TYPE_MX6SLEVK

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(1 * SZ_1M)

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

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */
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

#define CONFIG_PMIC_ROHM
#define CONFIG_PMIC_I2C
#define CONFIG_PMIC_FITIPOWER
/**************************************************************
 * Definition for IDME
***************************************************************/

#define CONFIG_CMD_IDME         1

#define CONFIG_IDME_UPDATE              1
#define CONFIG_IDME_UPDATE_MAGIC        "abcdefghhgfedcba"
#define CONFIG_IDME_UPDATE_MAGIC_SIZE   16


/* Command definition */
#include <config_cmd_default.h>

//#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY		3
#define CONFIG_LOADADDR			0x80800000
#define CONFIG_SYS_TEXT_BASE		0x982000

#define CONFIG_EANAB_EXTRA_ENV_SETTINGS \
	"boot_fdt=no\0" \
	"mmcdev="__stringify(CONFIG_MMC_DEFAULT_DEV)"\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"setmmcdev=mmc dev 1 0; \0"\
	"testboot=echo BOOTing from mmc ...; "\
		"booti mmc1\0 " \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev};" \
	"if mmc rescan; then " \
			"run testboot; " \
	"fi; " \

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		1024

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		256
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

//#define CONFIG_CMDLINE_EDITING
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
#define CONFIG_MMCROOT			"/dev/mmcblk0p1"  /* USDHC2 */

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
#define CONFIG_SYS_LDSCRIPT      "board/$(BOARDDIR)/u-boot-spl.lds"

#include <lab126/lab126_partitions_def.h>

#define CONFIG_CMD_MBOOT
#define CONFIG_MBOOTADDR         0x81000000

#define CONFIG_MBOOTCMD_LOCATION (CONFIG_MBOOTADDR - 0x70000)
#define CONFIG_MBOOTCMD_MAGIC    0xBC

#ifdef CONFIG_SYS_PROMPT
#undef CONFIG_SYS_PROMPT
#endif
#define CONFIG_SYS_PROMPT "spl > "

/* store aboot mode flag in either PMIC or SOC SNVC */
#define CONFIG_ABOOT_FLAG_PMIC

#endif				/* __CONFIG_H */
