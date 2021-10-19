/*
 * sign_of_life_mtk.c
 *
 * MTK platform implementation
 *
 * Copyright 2011 - 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 * Yang Liu (yangliu@lab126.com)
 * TODO: Add additional contributor's names.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform/mt_typedefs.h>
#include <platform/mt_rtc.h>
#include <platform/mt_pmic_wrap_init.h>
#if defined(UFBL_PROJ_ABC) || defined(UFBL_PROJ_ABC) || defined(UFBL_PROJ_ABC) || \
    defined(UFBL_PROJ_ABC) || defined(UFBL_PROJ_ABC) || defined(UFBL_PROJ_ABC)
#include <platform/mt_rtc_hw.h>
#endif
#include "life_cycle_reasons.h"

/* RTC Spare Register Definition */

/*
 * RTC_NEW_SPARE1: RTC_AL_DOM bit0~4
 * 	   bit 8 ~ 15 : reserved bits for boot reasons
 */
#define RTC_NEW_SPARE1_WARM_BOOT_KERNEL_PANIC    (1U << 8)
#define RTC_NEW_SPARE1_WARM_BOOT_KERNEL_WDOG     (1U << 9)
#define RTC_NEW_SPARE1_WARM_BOOT_HW_WDOG         (1U << 10)
#define RTC_NEW_SPARE1_WARM_BOOT_SW              (1U << 11)
#define RTC_NEW_SPARE1_COLD_BOOT_USB             (1U << 12)
#define RTC_NEW_SPARE1_COLD_BOOT_POWER_KEY       (1U << 13)
#define RTC_NEW_SPARE1_COLD_BOOT_POWER_SUPPLY    (1U << 14)

/*
 * RTC_NEW_SPARE2: RTC_AL_DOW bit0~2
 * 	   bit 8 ~ 15 : reserved bits
 */
#define RTC_NEW_SPARE2_SHUTDOWN_LONG_PWR_KEY_PRESS     (1U << 8)
#define RTC_NEW_SPARE2_SHUTDOWN_SW                     (1U << 9)
#define RTC_NEW_SPARE2_SHUTDOWN_PWR_KEY                (1U << 10)
#define RTC_NEW_SPARE2_SHUTDOWN_SUDDEN_PWR_LOSS        (1U << 11)
#define RTC_NEW_SPARE2_SHUTDOWN_UKNOWN                 (1U << 12)

/*
 * RTC_NEW_SPARE3: RTC_AL_MTH bit0~3
 * 	   bit 8 ~ 15 : reserved bits
 */
#define RTC_NEW_SPARE3_THERMAL_SHUTDOWN_BATTERY               (1U << 8)
#define RTC_NEW_SPARE3_THERMAL_SHUTDOWN_PMIC                  (1U << 9)
#define RTC_NEW_SPARE3_THERMAL_SHUTDOWN_SOC                   (1U << 10)
#define RTC_NEW_SPARE3_THERMAL_SHUTDOWN_MODEM                 (1U << 11)
#define RTC_NEW_SPARE3_THERMAL_SHUTDOWN_WIFI                  (1U << 12)
#define RTC_NEW_SPARE3_THERMAL_SHUTDOWN_PCB                   (1U << 13)

/*
 * RTC_SPAR0:
 *     bit 0 - 5 : SEC in power-on time
 *     bit 6 	 : 32K less bit. True:with 32K, False:Without 32K
 *     bit 9 - 15: reserved bits
 */
#define RTC_SPAR0_SPECIAL_MODE_LOW_BATTERY                    (1U << 9)
#define RTC_SPAR0_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED        (1U << 10)
#define RTC_SPAR0_SPECIAL_MODE_OTA                            (1U << 11)
#define RTC_SPAR0_SPECIAL_MODE_FACTORY_RESET                  (1U << 12)
#define RTC_SPAR0_SPECIAL_MODE_MASK                           0xfe00



static u16 rtc_read(u16 addr)
{
	u32 rdata = 0;

	pwrap_read((u32)addr, &rdata);
	return (u16)rdata;
}

static void rtc_write(u16 addr, u16 data)
{
	pwrap_write((u32)addr, (u32)data);
}

void rtc_busy_wait(void)
{
	u32 count = 0;
	do {
		while (rtc_read(RTC_BBPU) & RTC_BBPU_CBUSY) {
			if (count > 1000)
				break;
			mdelay(1);
			count++;

		}
	} while (0);
}

static void rtc_write_trigger(void)
{
	rtc_write(RTC_WRTGR, 1);
	rtc_busy_wait();
}

static int (mtk_read_boot_reason)(life_cycle_boot_reason *boot_reason)
{
	u16 rtc_breason;

	rtc_breason = rtc_read(RTC_AL_DOM);
	dprintf(CRITICAL, "%s: boot_reason is 0x%x\n", __func__, (u32)(rtc_breason));

	if (rtc_breason & RTC_NEW_SPARE1_WARM_BOOT_KERNEL_PANIC)
		*boot_reason = WARMBOOT_BY_KERNEL_PANIC;
	else if (rtc_breason & RTC_NEW_SPARE1_WARM_BOOT_KERNEL_WDOG)
		*boot_reason = WARMBOOT_BY_KERNEL_WATCHDOG;
	else if (rtc_breason & RTC_NEW_SPARE1_WARM_BOOT_HW_WDOG)
		*boot_reason = WARMBOOT_BY_HW_WATCHDOG;
	else if (rtc_breason & RTC_NEW_SPARE1_WARM_BOOT_SW)
		*boot_reason = WARMBOOT_BY_SW;
	else if (rtc_breason & RTC_NEW_SPARE1_COLD_BOOT_USB)
		*boot_reason = COLDBOOT_BY_USB;
	else if (rtc_breason & RTC_NEW_SPARE1_COLD_BOOT_POWER_KEY)
		*boot_reason = COLDBOOT_BY_POWER_KEY;
	else if (rtc_breason & RTC_NEW_SPARE1_COLD_BOOT_POWER_SUPPLY)
		*boot_reason = COLDBOOT_BY_POWER_SUPPLY;
	else {
		dprintf(CRITICAL, "Failed to read boot rtc boot reason\n");
		return -1;
	}

	return 0;
}

static int (mtk_write_boot_reason)(life_cycle_boot_reason boot_reason)
{
	u16 rtc_breason;

	rtc_breason = rtc_read(RTC_AL_DOM);

	dprintf(CRITICAL, "%s: current 0x%x boot_reason 0x%x\n", __func__, rtc_breason, boot_reason);
	if (boot_reason == WARMBOOT_BY_KERNEL_PANIC)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_WARM_BOOT_KERNEL_PANIC;
	else if (boot_reason == WARMBOOT_BY_KERNEL_WATCHDOG)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_WARM_BOOT_KERNEL_WDOG;
	else if (boot_reason == WARMBOOT_BY_HW_WATCHDOG)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_WARM_BOOT_HW_WDOG;
	else if (boot_reason == WARMBOOT_BY_SW)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_WARM_BOOT_SW;
	else if (boot_reason == COLDBOOT_BY_USB)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_COLD_BOOT_USB;
	else if (boot_reason == COLDBOOT_BY_POWER_KEY)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_COLD_BOOT_POWER_KEY;
	else if (boot_reason == COLDBOOT_BY_POWER_SUPPLY)
		rtc_breason = rtc_breason | RTC_NEW_SPARE1_COLD_BOOT_POWER_SUPPLY;

	rtc_write(RTC_AL_DOM, rtc_breason);
	rtc_write_trigger();

	return 0;
}

static int (mtk_read_shutdown_reason)(life_cycle_shutdown_reason *shutdown_reason)
{
	u16 rtc_shutdown_reason;

	rtc_shutdown_reason = rtc_read(RTC_AL_DOW);
	dprintf(CRITICAL, "%s: shutdown reason is 0x%x\n", __func__, rtc_shutdown_reason);
	if (rtc_shutdown_reason & RTC_NEW_SPARE2_SHUTDOWN_LONG_PWR_KEY_PRESS)
		*shutdown_reason = SHUTDOWN_BY_LONG_PWR_KEY_PRESS;
	else if (rtc_shutdown_reason & RTC_NEW_SPARE2_SHUTDOWN_SW)
		*shutdown_reason = SHUTDOWN_BY_SW;
	else if (rtc_shutdown_reason & RTC_NEW_SPARE2_SHUTDOWN_PWR_KEY)
		*shutdown_reason = SHUTDOWN_BY_PWR_KEY;
	else if (rtc_shutdown_reason & RTC_NEW_SPARE2_SHUTDOWN_SUDDEN_PWR_LOSS)
		*shutdown_reason = SHUTDOWN_BY_SUDDEN_POWER_LOSS;
	else if (rtc_shutdown_reason & RTC_NEW_SPARE2_SHUTDOWN_UKNOWN)
		*shutdown_reason = SHUTDOWN_BY_UNKNOWN_REASONS;
	else {
		dprintf(CRITICAL, "Failed to read boot rtc boot reason\n");
		return -1;
	}

	return 0;
}

static int (mtk_write_shutdown_reason)(life_cycle_shutdown_reason shutdown_reason)
{
	u16 rtc_shutdown_reason;

	rtc_shutdown_reason = rtc_read(RTC_AL_DOW);
	dprintf(CRITICAL, "%s: shutdown_reason 0x%x\n", __func__, rtc_shutdown_reason);

	if (shutdown_reason == SHUTDOWN_BY_LONG_PWR_KEY_PRESS)
		rtc_shutdown_reason = rtc_shutdown_reason | RTC_NEW_SPARE2_SHUTDOWN_LONG_PWR_KEY_PRESS;
	else if (shutdown_reason == SHUTDOWN_BY_SW)
		rtc_shutdown_reason = rtc_shutdown_reason | RTC_NEW_SPARE2_SHUTDOWN_SW;
	else if (shutdown_reason == SHUTDOWN_BY_PWR_KEY)
		rtc_shutdown_reason = rtc_shutdown_reason | RTC_NEW_SPARE2_SHUTDOWN_PWR_KEY;
	else if (shutdown_reason == SHUTDOWN_BY_SUDDEN_POWER_LOSS)
		rtc_shutdown_reason = rtc_shutdown_reason | RTC_NEW_SPARE2_SHUTDOWN_SUDDEN_PWR_LOSS;
	else if (shutdown_reason == SHUTDOWN_BY_UNKNOWN_REASONS)
		rtc_shutdown_reason = rtc_shutdown_reason | RTC_NEW_SPARE2_SHUTDOWN_UKNOWN;

	rtc_write(RTC_AL_DOW, rtc_shutdown_reason);
	rtc_write_trigger();
	return 0;
}

static int (mtk_read_thermal_shutdown_reason)(life_cycle_thermal_shutdown_reason *thermal_shutdown_reason)
{
	u16 rtc_thermal_shutdown_reason;

	rtc_thermal_shutdown_reason = rtc_read(RTC_AL_MTH);
	dprintf(CRITICAL, "%s: thermal shutdown reason 0x%x\n", __func__, rtc_thermal_shutdown_reason);

	if (rtc_thermal_shutdown_reason & RTC_NEW_SPARE3_THERMAL_SHUTDOWN_BATTERY)
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_BATTERY;
	else if (rtc_thermal_shutdown_reason & RTC_NEW_SPARE3_THERMAL_SHUTDOWN_PMIC)
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_PMIC;
	else if (rtc_thermal_shutdown_reason & RTC_NEW_SPARE3_THERMAL_SHUTDOWN_SOC)
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_SOC;
	else if (rtc_thermal_shutdown_reason & RTC_NEW_SPARE3_THERMAL_SHUTDOWN_MODEM)
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_MODEM;
	else if (rtc_thermal_shutdown_reason & RTC_NEW_SPARE3_THERMAL_SHUTDOWN_WIFI)
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_WIFI;
	else if (rtc_thermal_shutdown_reason & RTC_NEW_SPARE3_THERMAL_SHUTDOWN_PCB)
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_PCB;
	else {
		dprintf(CRITICAL, "Failed to read boot rtc boot reason\n");
		return -1;
	}

	return 0;
}

static int (mtk_write_thermal_shutdown_reason)(life_cycle_thermal_shutdown_reason thermal_shutdown_reason)
{
	u16 rtc_thermal_shutdown_reason;

	rtc_thermal_shutdown_reason = rtc_read(RTC_AL_MTH);
	dprintf(CRITICAL, "%s: shutdown_reason 0x%0x\n", __func__, rtc_thermal_shutdown_reason);


	if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_BATTERY)
		rtc_thermal_shutdown_reason = rtc_thermal_shutdown_reason | RTC_NEW_SPARE3_THERMAL_SHUTDOWN_BATTERY;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_PMIC)
		rtc_thermal_shutdown_reason = rtc_thermal_shutdown_reason | RTC_NEW_SPARE3_THERMAL_SHUTDOWN_PMIC;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_SOC)
		rtc_thermal_shutdown_reason = rtc_thermal_shutdown_reason | RTC_NEW_SPARE3_THERMAL_SHUTDOWN_SOC;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_MODEM)
		rtc_thermal_shutdown_reason = rtc_thermal_shutdown_reason | RTC_NEW_SPARE3_THERMAL_SHUTDOWN_MODEM;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_WIFI)
		rtc_thermal_shutdown_reason = rtc_thermal_shutdown_reason | RTC_NEW_SPARE3_THERMAL_SHUTDOWN_WIFI;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_PCB)
		rtc_thermal_shutdown_reason = rtc_thermal_shutdown_reason | RTC_NEW_SPARE3_THERMAL_SHUTDOWN_PCB;

	rtc_write(RTC_AL_MTH, rtc_thermal_shutdown_reason);
	rtc_write_trigger();
	return 0;
}

static int (mtk_read_special_mode)(life_cycle_special_mode *special_mode)
{
	u16 rtc_smode;

	rtc_smode = rtc_read(RTC_SPAR1);
	dprintf(CRITICAL, "%s: special mode is 0x%x\n", __func__, rtc_smode);
	if (rtc_smode & RTC_SPAR0_SPECIAL_MODE_LOW_BATTERY)
		*special_mode = LIFE_CYCLE_SMODE_LOW_BATTERY;
	else if (rtc_smode & RTC_SPAR0_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED)
		*special_mode = LIFE_CYCLE_SMODE_WARM_BOOT_USB_CONNECTED;
	else if (rtc_smode & RTC_SPAR0_SPECIAL_MODE_OTA)
		*special_mode = LIFE_CYCLE_SMODE_OTA;
	else if (rtc_smode & RTC_SPAR0_SPECIAL_MODE_FACTORY_RESET)
		*special_mode = LIFE_CYCLE_SMODE_FACTORY_RESET;
	else {
		dprintf(CRITICAL, "Failed to read boot rtc boot reason\n");
		return -1;
	}
	return 0;
}

static int (mtk_write_special_mode)(life_cycle_special_mode special_mode)
{
	u16 rtc_smode;

	rtc_smode = rtc_read(RTC_SPAR1);
	dprintf(CRITICAL, "%s: special_mode 0x%x\n", __func__, rtc_smode);

	if (special_mode == LIFE_CYCLE_SMODE_LOW_BATTERY)
		rtc_smode = rtc_smode | RTC_SPAR0_SPECIAL_MODE_LOW_BATTERY;
	else if (special_mode == LIFE_CYCLE_SMODE_WARM_BOOT_USB_CONNECTED)
		rtc_smode = rtc_smode | RTC_SPAR0_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED;
	else if (special_mode == LIFE_CYCLE_SMODE_OTA)
		rtc_smode = rtc_smode | RTC_SPAR0_SPECIAL_MODE_OTA;
	else if (special_mode == LIFE_CYCLE_SMODE_FACTORY_RESET)
		rtc_smode = rtc_smode | RTC_SPAR0_SPECIAL_MODE_FACTORY_RESET;

	rtc_write(RTC_SPAR1, rtc_smode);
	rtc_write_trigger();
	return 0;
}


int life_cycle_platform_init(life_cycle_reasons_ops *sol_ops)
{
	dprintf(CRITICAL, "%s: Support MTK platform\n", __func__);
	sol_ops->read_boot_reason = mtk_read_boot_reason;
	sol_ops->write_boot_reason = mtk_write_boot_reason;
	sol_ops->read_shutdown_reason = mtk_read_shutdown_reason;
	sol_ops->write_shutdown_reason = mtk_write_shutdown_reason;
	sol_ops->read_thermal_shutdown_reason = mtk_read_thermal_shutdown_reason;
	sol_ops->write_thermal_shutdown_reason = mtk_write_thermal_shutdown_reason;
	sol_ops->read_special_mode = mtk_read_special_mode;
	sol_ops->write_special_mode = mtk_write_special_mode;

	return 0;
}
