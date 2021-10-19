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

#define LPM_DSM_MODE	  1
#define LPM_STANDBY_MODE  2
#define LPM_STOP_MODE	  3
#define LPM_IDLE_MODE	  4

#define LPM_LAB126_IDLE_MODE    	5
#define LPM_LAB126_SUSPEND_MODE 	6
#define LPM_LAB126_IDLE_WAIT_MODE    	7
#define LPM_LAB126_IDLE_1_MODE    	8
#define LPM_LAB126_IDLE_2_MODE    	9

#define LPM_CFG_ID_DRAM_EN               (0)
#define LPM_CFG_ID_L1_CACHE_EN           (LPM_CFG_ID_DRAM_EN           +1)
#define LPM_CFG_ID_L2_CACHE_EN           (LPM_CFG_ID_L1_CACHE_EN       +1)
#define LPM_CFG_ID_ARM_CLOCK_FREQ        (LPM_CFG_ID_L2_CACHE_EN       +1)
#define LPM_CFG_ID_BUS_CLOCK_FREQ        (LPM_CFG_ID_ARM_CLOCK_FREQ    +1)
#define LPM_CFG_ID_PLL_ARM_BYPASS        (LPM_CFG_ID_BUS_CLOCK_FREQ    +1)
#define LPM_CFG_ID_PLL_528_BYPASS        (LPM_CFG_ID_PLL_ARM_BYPASS    +1)
#define LPM_CFG_ID_PLL_480_BYPASS        (LPM_CFG_ID_PLL_528_BYPASS    +1)
#define LPM_CFG_ID_ARM_VOLTAGE           (LPM_CFG_ID_PLL_480_BYPASS    +1)
#define LPM_CFG_ID_SOC_VOLTAGE           (LPM_CFG_ID_ARM_VOLTAGE       +1)
#define LPM_CFG_ID_PU_VOLTAGE            (LPM_CFG_ID_SOC_VOLTAGE       +1)
#define LPM_CFG_ID_DISPLAY_PG            (LPM_CFG_ID_PU_VOLTAGE        +1)
#define LPM_CFG_ID_USE_RC_OSC            (LPM_CFG_ID_DISPLAY_PG        +1)
#define LPM_CFG_ID_LOW_POWER_MODE        (LPM_CFG_ID_USE_RC_OSC        +1)
#define LPM_CFG_ID_ARM_POWER_GATE_EN     (LPM_CFG_ID_LOW_POWER_MODE    +1)
#define LPM_CFG_ID_ARM_CLOCK_GATE_EN     (LPM_CFG_ID_ARM_POWER_GATE_EN +1)
#define LPM_CFG_ID_L2_POWER_GATE_EN      (LPM_CFG_ID_ARM_CLOCK_GATE_EN +1)
#define LPM_CFG_ID_SOC_RWB_EN            (LPM_CFG_ID_L2_POWER_GATE_EN  +1)
#define LPM_CFG_ID_OSC_POWER_DOWN_EN     (LPM_CFG_ID_SOC_RWB_EN        +1)
#define LPM_CFG_ID_STANDBY_VOLTAGE_EN    (LPM_CFG_ID_OSC_POWER_DOWN_EN +1)
#define LPM_CFG_ID_WEAK_2P5_EN           (LPM_CFG_ID_STANDBY_VOLTAGE_EN+1)
#define LPM_CFG_ID_RING_OSC_EN           (LPM_CFG_ID_WEAK_2P5_EN       +1)
#define LPM_CFG_ID_STOP_MODE_CONFIG      (LPM_CFG_ID_RING_OSC_EN       +1)
#define LPM_CFG_ID_RBC_EN                (LPM_CFG_ID_STOP_MODE_CONFIG  +1)
#define LPM_CFG_ID_OSC_COUNT             (LPM_CFG_ID_RBC_EN            +1)
#define LPM_CFG_ID_RBC_COUNT             (LPM_CFG_ID_OSC_COUNT         +1)
#define LPM_CFG_ID_STBY_COUNT            (LPM_CFG_ID_RBC_COUNT         +1)
#define LPM_CFG_ID_ARM_PUP_SW            (LPM_CFG_ID_STBY_COUNT        +1)
#define LPM_CFG_ID_ARM_PUP_SW2ISO        (LPM_CFG_ID_ARM_PUP_SW        +1)
#define LPM_CFG_ID_RC_OSC_EN             (LPM_CFG_ID_ARM_PUP_SW2ISO    +1)
#define LPM_CFG_ID_RC_OSC_CG_OVERRIDE    (LPM_CFG_ID_RC_OSC_EN         +1)
#define LPM_CFG_ID_GPT_WAKEUP_EN         (LPM_CFG_ID_RC_OSC_CG_OVERRIDE+1)
#define LPM_CFG_ID_GPT_WAKEUP_TIME_MS    (LPM_CFG_ID_GPT_WAKEUP_EN     +1)
#define LPM_CFG_ID_UART_WAKEUP_EN        (LPM_CFG_ID_GPT_WAKEUP_TIME_MS+1)
#define LPM_CFG_ID_WAKEUP_LOOP           (LPM_CFG_ID_UART_WAKEUP_EN    +1)
#define LPM_CFG_ID_MAX                   (LPM_CFG_ID_WAKEUP_LOOP         )

void lpm_disable_phys(void);
void lpm_disable_unused_clocks(void);
void lpm_analog_low_power_setting(void);

void lpm_enter_exit(void);
void lpm_config_init(void);
void lpm_config_mode(unsigned int mode);
