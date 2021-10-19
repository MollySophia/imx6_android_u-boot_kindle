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
void system_config_update(void);
void system_config_init(void);
void system_enter_lpm(void);
int system_config_pu_voltage(unsigned int voltage);
int system_config_soc_voltage(unsigned int voltage);
int system_config_arm_voltage(unsigned int voltage);
int system_config_arm_pll_bypass(unsigned int bypass);
int system_config_528_pll_bypass(unsigned int bypass);
int system_config_480_pll_bypass(unsigned int bypass);
int system_config_dram(unsigned int enable);
int system_config_arm_clock(unsigned int freq);
int system_config_display_pg(unsigned int enable);
int system_config_use_rc_osc(unsigned int enable);

#define SYS_CFG_ID_DRAM_EN               (0)
#define SYS_CFG_ID_L1_CACHE_EN           (SYS_CFG_ID_DRAM_EN + 1)
#define SYS_CFG_ID_L2_CACHE_EN           (SYS_CFG_ID_L1_CACHE_EN + 1)
#define SYS_CFG_ID_ARM_CLOCK_FREQ        (SYS_CFG_ID_L2_CACHE_EN + 1)
#define SYS_CFG_ID_BUS_CLOCK_FREQ        (SYS_CFG_ID_ARM_CLOCK_FREQ + 1)
#define SYS_CFG_ID_PLL_ARM_BYPASS        (SYS_CFG_ID_BUS_CLOCK_FREQ + 1)
#define SYS_CFG_ID_PLL_528_BYPASS        (SYS_CFG_ID_PLL_ARM_BYPASS + 1)
#define SYS_CFG_ID_PLL_480_BYPASS        (SYS_CFG_ID_PLL_528_BYPASS + 1)
#define SYS_CFG_ID_ARM_VOLTAGE           (SYS_CFG_ID_PLL_480_BYPASS + 1)
#define SYS_CFG_ID_SOC_VOLTAGE           (SYS_CFG_ID_ARM_VOLTAGE + 1)
#define SYS_CFG_ID_PU_VOLTAGE            (SYS_CFG_ID_SOC_VOLTAGE + 1)
#define SYS_CFG_ID_DISPLAY_PG            (SYS_CFG_ID_PU_VOLTAGE + 1)
#define SYS_CFG_ID_USE_RC_OSC            (SYS_CFG_ID_DISPLAY_PG + 1)
#define SYS_CFG_ID_MAX                   (SYS_CFG_ID_USE_RC_OSC)

#define GLOBAL_L1_CACHE_EN   1
#define GLOBAL_L2_CACHE_EN   1
#define GLOBAL_DRAM_EN       1

#define dram_enter_lpm()     lpddr2_enter_lpm()
