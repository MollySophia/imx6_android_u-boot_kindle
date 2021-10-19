/*
 * pmic.h 
 *
 * Copyright 2013 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __PMIC_H__
#define __PMIC_H__

typedef enum {
    pmic_charge_none = 0,
    pmic_charge_host,
    pmic_charge_usb_high,
    pmic_charge_third_party,
    pmic_charge_wall,
} pmic_charge_type;

#define BATTERY_INVALID_VOLTAGE			0xFFFF

#define PMIC_REG_DECODE(val, ad, rg) (ad) = (((val) >> 8) & 0xFF); (rg) = ((val) & 0xFF) 

int pmic_set_alarm(unsigned int secs_from_now);
int pmic_enable_green_led(int enable);
int pmic_adc_read_last_voltage(unsigned short *result);
int pmic_adc_read_voltage(unsigned short *result);
int pmic_charging(void);
int pmic_charge_restart(void);
int pmic_set_autochg(int autochg);
int pmic_set_chg_current(pmic_charge_type chg_value);
int pmic_start_charging(pmic_charge_type chg_value, int autochg);
int pmic_power_off(void);
int pmic_reset(void);
int pmic_init(void);
void pmic_zforce2_pwrenable(int);
extern int pmic_fg_read_voltage(unsigned short *voltage);
extern int pmic_fg_read_avg_current(int *curr);
extern int pmic_fg_read_capacity(unsigned short *capacity);
extern int pmic_fg_read_temperature(int *temperature);
extern int pmic_charger_set_current(int current);
extern int pmic_fl_set (int brightness);
extern int board_pmic_init(void);
extern int pmic_vcom_set(char* vcom_val);
extern int pmic_enable_vcom(int enable);
extern int pmic_enable_display_power_rail(unsigned int enable);
extern int pmic_read_epdc_temperature(int *temperature);

#if defined(CONFIG_PMIC_I2C)
int pmic_wor_reg(unsigned char saddr, unsigned int reg, const unsigned int value, const unsigned int mask);
extern int board_pmic_read_reg(unsigned char saddr, unsigned int reg_num, unsigned int *reg_val);
extern int board_pmic_write_reg(unsigned char saddr, unsigned int reg_num, const unsigned int reg_val);
extern int board_pmic_write_verify_reg(unsigned char saddr, unsigned int reg_num, const unsigned int reg_val);
extern int board_pmic_get_aboot_mode(void);
extern void board_pmic_set_aboot_mode(u32 mode);
#ifdef CONFIG_FASTBOOT_MAGIC_SEQ
extern int board_fastboot_keys_pressed(void);
#endif
#elif defined(CONFIG_PMIC_SPI)
int pmic_wor_reg(int reg, const unsigned int value, const unsigned int mask);
extern int board_pmic_read_reg(int reg_num, unsigned int *reg_val);
extern int board_pmic_write_reg(int reg_num, const unsigned int reg_val);
extern int board_pmic_write_verify_reg(int reg_num, const unsigned int reg_val);
#else
#error must define PMIC transport
#endif

extern int board_pmic_reset(void);
extern int board_enable_green_led(int enable);

#if CONFIG_MX6_INTER_LDO_BYPASS
extern void bypass_internal_pmic_ldo(void);
#endif

#define pmic_read_reg board_pmic_read_reg
#define pmic_write_reg board_pmic_write_reg
#define pmic_write_verify_reg board_pmic_write_verify_reg
#endif
