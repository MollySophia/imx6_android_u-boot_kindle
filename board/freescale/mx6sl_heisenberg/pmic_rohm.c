/*
 * pmic.c 
 *
 * Copyright 2012 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file pmic.c
 * @brief This file contains mx6 board-specific info to talk to the MAXIM 77697 PMIC.
 *
 */

#define __DEBUG_PMIC__ 1
#ifdef __DEBUG_PMIC__
#define DBG(fmt,args...)\
		serial_printf("[%s] %s:%d: "fmt,\
				__FILE__,__FUNCTION__,__LINE__, ##args)
#else
#define DBG(fmt,args...)
#endif


#include <common.h>

#include <lab126/pmic.h>
#include <lab126/idme.h>
#include <i2c.h>
#include "regs-anadig.h"
#include <bcd.h>
#include <rtc.h>

#ifdef CONFIG_PMIC_MAX77696
#include <lab126/pmic_max77696.h>
#endif

#ifdef CONFIG_PMIC_ROHM
#include <lab126/pmic_rohm.h>
#endif

#ifdef CONFIG_PMIC_FITIPOWER
#include <lab126/pmic_fitipower.h>
#endif
#include <asm/arch/sys_proto.h>


int board_pmic_init(void) 
{
    int ret;
    unsigned int reg = 0;

    /* Make sure the ROHM PMIC is NOT in test mode */
    /* unlock the test mode with these magic sequences */
    ret = board_pmic_write_reg(ROHM_I2C_ADDR, 0xFE, 0x76);
    ret += board_pmic_write_reg(ROHM_I2C_ADDR, 0xFE, 0x66);
    ret += board_pmic_write_reg(ROHM_I2C_ADDR, 0xFE, 0x56);
    ret += board_pmic_read_reg(ROHM_I2C_ADDR, 0xFE, &reg);

    /* if all above 4 read/write reg. ops are finished and
     * we are in test mode (0xFF) */
    if (ret == 4 && reg == 0xFF) {
       /* Make sure the PMIC is in sane states */
       /* Make sure the PMIC is NOT in accelerate mode */
       board_pmic_write_verify_reg(ROHM_I2C_ADDR, 0xF1, 0x00);

       /* Make sure the power seq. register is in NORMAL mode (0x07) */
       board_pmic_write_verify_reg(ROHM_I2C_ADDR, 0x55, 0x07);
    } else
       DBG("Failed to enter PMIC test mode\n");

    /* lock the test mode unconditionally so we are back to normal mode (0x0).
     * try again if failed */
    if (board_pmic_write_verify_reg(ROHM_I2C_ADDR, 0xFE, 0x0))
       board_pmic_write_reg(ROHM_I2C_ADDR, 0xFE, 0x0);

    return 1;
}

int board_pmic_write_reg(unsigned char saddr, unsigned int reg, const unsigned int val) 
{
    int ret;

    i2c_set_bus_num(0);
    switch(saddr) {
#ifdef CONFIG_PMIC_MAX77696
    case FG_SADDR: 
      ret = i2c_write(saddr, reg, 1, (unsigned char *) &val, 2);
    break;
#endif
    default:
      ret = i2c_write(saddr, reg, 1, (unsigned char *) &val, 1);
    break;
    }

    if (ret)
    {
        DBG("\n%s: failed! for sddr %x, regaddr: %x \n", __func__, saddr, reg);
	return 0;
    }

    return 1;
}

int board_pmic_read_reg(unsigned char saddr, unsigned int reg, unsigned int *val) 
{
    int ret;

    *val = 0;

    i2c_set_bus_num(0);
    switch(saddr) {
#ifdef CONFIG_PMIC_MAX77696
    case FG_SADDR:
      ret = i2c_read(saddr, reg, 1, (unsigned char *) val, 2);
    break;
#endif
    default:
      ret = i2c_read(saddr, reg, 1, (unsigned char *) val, 1);
    break;
    }
    if (ret)
    {
        DBG("\n%s: failed! for sddr %x, regaddr: %x \n", __func__, saddr, reg);
	return 0;
    }

    return 1;
}

int board_pmic_write_verify_reg(unsigned char saddr, unsigned int reg, unsigned int val)
{
   unsigned int rdval; 

   board_pmic_write_reg(saddr, reg, val);
   udelay(100); //some delay before reading back
   board_pmic_read_reg(saddr, reg, &rdval); 

   if(rdval != val) {
        DBG("\n%s: WriteVerify failed! for sddr %x, regaddr: %x		\
		Value written %x, value read-back %x\n", __func__, saddr, reg, val, rdval);
   	return 1;
   }
   return 0;
}

int board_enable_green_led(int enable) 
{
#if 0	/* BEN TODO */
    int pad_val = PAD_CTL_DRV_NORMAL | PAD_CTL_SRE_SLOW | PAD_CTL_DRV_1_8V |
	    PAD_CTL_HYS_CMOS | PAD_CTL_PKE_NONE | PAD_CTL_100K_PU;

    mx35_gpio_direction(IOMUX_TO_GPIO(MX35_PIN_CSI_D12), MX35_GPIO_DIRECTION_OUT);

    /* shasta/luigi controls led w/ gpio */
    if (enable) {
	mx35_gpio_set(IOMUX_TO_GPIO(MX35_PIN_CSI_D12), 1);
    } else {
	mx35_gpio_set(IOMUX_TO_GPIO(MX35_PIN_CSI_D12), 0);
    }

    mxc_iomux_set_pad(MX35_PIN_CSI_D12, pad_val);
#endif
    return 1;
}

#ifdef CONFIG_MX6_INTER_LDO_BYPASS
#define REG_RD(base, reg) \
    (*(volatile unsigned int *)((base) + (reg)))
#define REG_WR(base, reg, value) \
    ((*(volatile unsigned int *)((base) + (reg))) = (value))


void bypass_internal_pmic_ldo(void)
{
    unsigned int val = 0;

    /* Bypass the VDDSOC from Anatop */
    val = REG_RD(ANATOP_BASE_ADDR, HW_ANADIG_REG_CORE);
    val &= ~BM_ANADIG_REG_CORE_REG2_TRG;
    val |= BF_ANADIG_REG_CORE_REG2_TRG(0x1f);
    REG_WR(ANATOP_BASE_ADDR, HW_ANADIG_REG_CORE, val);

    /* Bypass the VDDCORE from Anatop */
    val = REG_RD(ANATOP_BASE_ADDR, HW_ANADIG_REG_CORE);
    val &= ~BM_ANADIG_REG_CORE_REG0_TRG;
    val |= BF_ANADIG_REG_CORE_REG0_TRG(0x1f);
    REG_WR(ANATOP_BASE_ADDR, HW_ANADIG_REG_CORE, val);

    /* Powergate the VDDPU from Anatop - unused on abc123 */
    val = REG_RD(ANATOP_BASE_ADDR, HW_ANADIG_REG_CORE);
    val &= ~BM_ANADIG_REG_CORE_REG1_TRG;
    val |= BF_ANADIG_REG_CORE_REG1_TRG(0x00);
    REG_WR(ANATOP_BASE_ADDR, HW_ANADIG_REG_CORE, val);
}
#endif

int board_pmic_reset(void)
{
    return 1;
}

static u32 aboot_mode = 0xFFFF;
#define PMIC_BOOT_REGISTER 0xB7
int board_pmic_get_aboot_mode(void)
{
	u32 reg;
	int ret = 1;
	int retry = 10;

	if (aboot_mode != 0xFFFF)
		return aboot_mode;

	do {
		ret = board_pmic_read_reg(0x4B, PMIC_BOOT_REGISTER, &reg);
	} while ((ret == 0) && (retry-- > 0));
	
	aboot_mode = reg & ANDROID_BOOT_MASK;

	/* The comment below is not true anymore. Software Reset flag
	 * uses register B0 and not B7 which is only used for aboot mode
	 */
	/*
	 * Donot clear 'regular reboot' reason, this is set by
	 * kernel and is used to distinguish between watchdog
	 * reset and a normal reboot.
	 */
	if (aboot_mode == ANDROID_BOOT_REGULAR_REBOOT)
		return aboot_mode;

	reg &= ~ANDROID_BOOT_MASK;
#ifndef CONFIG_BIST
	if (aboot_mode == ANDROID_BOOT_FASTBOOT)
		reg |= ANDROID_BOOT_FASTBOOT;
#endif
	ret = board_pmic_write_reg(0x4B, PMIC_BOOT_REGISTER, reg);

	return aboot_mode;
}

void board_pmic_set_aboot_mode(u32 mode)
{
	u32 reg;
	int ret;

	ret = board_pmic_read_reg(0x4B, PMIC_BOOT_REGISTER, &reg);
	reg &= ~ANDROID_BOOT_MASK;
	reg |= (mode & ANDROID_BOOT_MASK);
	ret = board_pmic_write_reg(0x4B, PMIC_BOOT_REGISTER, reg);
}

/*
 * Read HW RTC time and convert to Linux time.
 * The HW values are BCD encoded.
 * - Months are 1..12 vs Linux 0-11
 * - Years are 0..99 vs Linux 1900..N (we assume 21st century)
 */
int pmic_read_rtc_time(struct rtc_time* tm) {
	u32 sec, min, hour, day, mon, year;

	if (!tm)
		return -1;

	if (!board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_SEC, &sec)   ||
	    !board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_MIN, &min)   ||
	    !board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_HOUR, &hour) ||
	    !board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_DAY, &day)   ||
	    !board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_MONTH, &mon) ||
	    !board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_YEAR, &year))
		return -1;

	hour = hour & ~HOUR_24HOUR;
	tm->tm_sec = bcd2bin(sec);
	tm->tm_min = bcd2bin(min);
	tm->tm_hour = bcd2bin(hour);
	tm->tm_mday = bcd2bin(day);
	tm->tm_mon = bcd2bin(mon) - 1;
	tm->tm_year = bcd2bin(year) + 100;

	return 0;
}

#ifdef CONFIG_FASTBOOT_MAGIC_SEQ

/*
 * The fastboot magic sequence logic goes like this.
 * Use will need to hold the power button trigger a reset for a total of
 * LP_FASTBOOT_MAX_CNT times and each time within LP_FASTBOOT_THRESHOLD seconds
 */
#define LP_FASTBOOT_THRESHOLD   15 /* wait time between resets in seconds */
#define LP_FASTBOOT_MAX_CNT     2  /* number of resets require to go fastboot */

static int get_lp_press_time(void)
{
	struct rtc_time tm;

	if (pmic_read_rtc_time(&tm))
		return 0;

	/*
	 * Drop the year/month/day for time stamp calculation.
	 * The collision should be low in which user presses LP a year/month apart.
	 */
	return (tm.tm_hour*60 + tm.tm_min)*60 + tm.tm_sec;
}

/*
 * To enter fastboot with power button directly.
 * Hold the power button to go into the reset sequence LP_FASTBOOT_MAX_CNT times. Each reset has to
 * be within LP_FASTBOOT_THRESHOLD interval.
 */
int board_fastboot_keys_pressed(void)
{
	u32 reg = 0;
	char lpstat[BOARD_LPSTAT_LEN+1];
	u32 nowt, prevt = 0;
	u32 cnt = 1;
	int ret = 0;
	u32 save = 0;

#define SOFT_REBOOT   0xA5
#define LP_CNT_SHIFT  28 /* # bits used for storing long press timestamp */
#define LP_CNT(x)     (x >> LP_CNT_SHIFT)
#define LP_STAMP(x)   (x & 0x0FFFFFFF)
#define BIT(x)        (0x1 << x)

	if (!board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_RESERVED_0, &reg))
		return 0;

	if (reg != SOFT_REBOOT) {
		if (!board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_INT_STAT_03, &reg))
			return 0;
		/* check for long press */
		if (reg & BIT(2)) {
			nowt = get_lp_press_time();
			if (!nowt)
				return 0;

			/* if lpstat idme var is set */
			if (!idme_get_var("lpstat", lpstat, sizeof(lpstat))) {
				save = simple_strtoul(lpstat, NULL, 10);
				prevt = LP_STAMP(save);
				cnt = LP_CNT(save);
			}

			/* if back to back long pressed power button is detected */
			u32 diff = nowt - prevt;
			printf("LPstat now=%d prevt=%d diff=%u cnt=%d\n",
			       nowt, prevt, diff, cnt);
			if (diff && diff <= LP_FASTBOOT_THRESHOLD) {
				cnt++;
				if (cnt >= LP_FASTBOOT_MAX_CNT) {
					printf("Fastboot combination pressed, go into FastBootMode\n");
					ret = 1;
					/* reset stats */
					cnt = 0;
					nowt = 0;
				}
			} else
				cnt = 1; /* reset the count */

			/* updates last long press stats */
			save = cnt;
			save = (save << LP_CNT_SHIFT) | nowt;
			snprintf(lpstat, sizeof(lpstat), "%u", save);
			idme_update_var_with_size("lpstat", lpstat, sizeof(lpstat));
		}
	}

	return ret;
}

/*
 * device_powered_of().
 * return true if device was powered off by checking reset code == 0x55
 * This is written by the Linux PMIC driver side.
 */
bool device_powered_off(void)
{
	u32 reg = 0;

#define POWEROFF 0x55
	if (!board_pmic_read_reg(ROHM_I2C_ADDR, BD7181X_REG_RESERVED_0, &reg))
		return false;
	return reg == POWEROFF;
}

#endif /* CONFIG_FASTBOOT_MAGIC_SEQ */
