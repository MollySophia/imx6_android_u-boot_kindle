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

#include "config.h"
#include "dram_api.h"
#include "system_config.h"
#include "low_power_mode.h"

#ifdef CONFIG_PMIC
#include <lab126/pmic.h>
#endif

#ifdef CONFIG_PMIC_MAX77696
#include <lab126/pmic_max77696.h>
#endif

#define SYS_CFG_DEFAULT_DRAM_EN               GLOBAL_DRAM_EN
#define SYS_CFG_DEFAULT_L1_CACHE_EN           GLOBAL_L1_CACHE_EN
#define SYS_CFG_DEFAULT_L2_CACHE_EN           GLOBAL_L2_CACHE_EN
#define SYS_CFG_DEFAULT_ARM_CLOCK_FREQ        800
#define SYS_CFG_DEFAULT_BUS_CLOCK_FREQ        400
#define SYS_CFG_DEFAULT_PLL_ARM_BYPASS        0
#define SYS_CFG_DEFAULT_PLL_528_BYPASS        0
#define SYS_CFG_DEFAULT_PLL_480_BYPASS        0
#define SYS_CFG_DEFAULT_ARM_VOLTAGE           1100
#define SYS_CFG_DEFAULT_SOC_VOLTAGE           1100
#define SYS_CFG_DEFAULT_PU_VOLTAGE            0
#define SYS_CFG_DEFAULT_DISPLAY_PG            1
#define SYS_CFG_DEFAULT_USE_RC_OSC            0

config_t system_config[] __attribute__ ((section (".low_power_data")))=
{
	{ SYS_CFG_ID_DRAM_EN           , "DRAM_EN            ", "", SYS_CFG_DEFAULT_DRAM_EN           , 0, 0, 0, system_config_dram, },
	{ SYS_CFG_ID_L1_CACHE_EN       , "L1_CACHE_EN        ", "", SYS_CFG_DEFAULT_L1_CACHE_EN       , 0, 0, 0, 0, },
	{ SYS_CFG_ID_L2_CACHE_EN       , "L2_CACHE_EN        ", "", SYS_CFG_DEFAULT_L2_CACHE_EN       , 0, 0, 0, 0, },
	{ SYS_CFG_ID_ARM_CLOCK_FREQ    , "ARM_CLOCK_FREQ     ", "", SYS_CFG_DEFAULT_ARM_CLOCK_FREQ    , 0, 0, 0, system_config_arm_clock, },
	{ SYS_CFG_ID_BUS_CLOCK_FREQ    , "BUS_CLOCK_FREQ     ", "", SYS_CFG_DEFAULT_BUS_CLOCK_FREQ    , 0, 0, 0, 0, },
	{ SYS_CFG_ID_PLL_ARM_BYPASS    , "PLL_ARM_BYPASS     ", "", SYS_CFG_DEFAULT_PLL_ARM_BYPASS    , 0, 0, 0, system_config_arm_pll_bypass, },
	{ SYS_CFG_ID_PLL_528_BYPASS    , "PLL_528_BYPASS     ", "", SYS_CFG_DEFAULT_PLL_528_BYPASS    , 0, 0, 0, system_config_528_pll_bypass, },
	{ SYS_CFG_ID_PLL_480_BYPASS    , "PLL_480_BYPASS     ", "", SYS_CFG_DEFAULT_PLL_480_BYPASS    , 0, 0, 0, system_config_480_pll_bypass, },
	{ SYS_CFG_ID_ARM_VOLTAGE       , "ARM_VOLTAGE        ", "", SYS_CFG_DEFAULT_ARM_VOLTAGE       , 0, 0, 0, system_config_arm_voltage, },
	{ SYS_CFG_ID_SOC_VOLTAGE       , "SOC_VOLTAGE        ", "", SYS_CFG_DEFAULT_SOC_VOLTAGE       , 0, 0, 0, system_config_soc_voltage, },
	{ SYS_CFG_ID_PU_VOLTAGE        , "PU_VOLTAGE         ", "", SYS_CFG_DEFAULT_PU_VOLTAGE        , 0, 0, 0, system_config_pu_voltage, },
	{ SYS_CFG_ID_DISPLAY_PG        , "DISPLAY_PG         ", "", SYS_CFG_DEFAULT_DISPLAY_PG        , 0, 0, 0, system_config_display_pg, },
	{ SYS_CFG_ID_USE_RC_OSC        , "USE_RC_OSC         ", "", SYS_CFG_DEFAULT_USE_RC_OSC        , 0, 0, 0, system_config_use_rc_osc },
};

#ifdef CFG_FROM_DEFINE
#define DRAM_EN               SYS_CFG_DEFAULT_DRAM_EN
#define L1_CACHE_EN           SYS_CFG_DEFAULT_L1_CACHE_EN
#define L2_CACHE_EN           SYS_CFG_DEFAULT_L2_CACHE_EN
#define ARM_CLOCK_FREQ        SYS_CFG_DEFAULT_ARM_CLOCK_FREQ
#define BUS_CLOCK_FREQ        SYS_CFG_DEFAULT_BUS_CLOCK_FREQ
#define PLL_ARM_BYPASS        SYS_CFG_DEFAULT_PLL_ARM_BYPASS
#define PLL_528_BYPASS        SYS_CFG_DEFAULT_PLL_528_BYPASS
#define PLL_480_BYPASS        SYS_CFG_DEFAULT_PLL_480_BYPASS
#define ARM_VOLTAGE           SYS_CFG_DEFAULT_ARM_VOLTAGE
#define SOC_VOLTAGE           SYS_CFG_DEFAULT_SOC_VOLTAGE
#define PU_VOLTAGE            SYS_CFG_DEFAULT_PU_VOLTAGE
#define DISPLAY_PG            SYS_CFG_DEFAULT_DISPLAY_PG
#define USE_RC_OSC            SYS_CFG_DEFAULT_USE_RC_OSC
#else
#define DRAM_EN               (system_config[SYS_CFG_ID_DRAM_EN           ].parameter0)
#define L1_CACHE_EN           (system_config[SYS_CFG_ID_L1_CACHE_EN       ].parameter0)
#define L2_CACHE_EN           (system_config[SYS_CFG_ID_L2_CACHE_EN       ].parameter0)
#define ARM_CLOCK_FREQ        (system_config[SYS_CFG_ID_ARM_CLOCK_FREQ    ].parameter0)
#define BUS_CLOCK_FREQ        (system_config[SYS_CFG_ID_BUS_CLOCK_FREQ    ].parameter0)
#define PLL_ARM_BYPASS        (system_config[SYS_CFG_ID_PLL_ARM_BYPASS    ].parameter0)
#define PLL_528_BYPASS        (system_config[SYS_CFG_ID_PLL_528_BYPASS    ].parameter0)
#define PLL_480_BYPASS        (system_config[SYS_CFG_ID_PLL_480_BYPASS    ].parameter0)
#define ARM_VOLTAGE           (system_config[SYS_CFG_ID_ARM_VOLTAGE       ].parameter0)
#define SOC_VOLTAGE           (system_config[SYS_CFG_ID_SOC_VOLTAGE       ].parameter0)
#define PU_VOLTAGE            (system_config[SYS_CFG_ID_PU_VOLTAGE        ].parameter0)
#define DISPLAY_PG            (system_config[SYS_CFG_ID_DISPLAY_PG        ].parameter0)
#define USE_RC_OSC            (system_config[SYS_CFG_ID_USE_RC_OSC        ].parameter0)
#endif

extern unsigned long current_lpm_config;

unsigned int voltage_to_reg(unsigned int)
	__attribute__ ((section (".low_power_code")));
unsigned int voltage_to_reg(unsigned int voltage)
{
	if (voltage > 1450)
		return 0x1F;  // Bypass Mode
	else if (voltage < 725)
		return 0x0;  // Power Gate Mode
	else
		return ((voltage - 725) / 25 + 1); // Regulation Mode
}

unsigned int reg_to_voltage(unsigned int)
	__attribute__ ((section (".low_power_code")));
unsigned int reg_to_voltage(unsigned int reg)
{
	if (reg == 0x1F)
		return 1500;  // Bypass Mode
	else if (reg == 0x0)
		return 0;     // Power Gate Mode
	else
		return (reg - 1) * 25 + 725; // Regulation Mode
}

int voltage_checking(unsigned int)
	__attribute__ ((section (".low_power_code")));
int voltage_checking(unsigned int voltage)
{
	if (((voltage > 1450) && (voltage != 1500)) || ((voltage < 725) && (voltage != 0)))
		return 1;
	else if ((voltage % 25) != 0)
		return 2;
	else
		return 0;
}

int system_config_pu_voltage(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_pu_voltage(unsigned int voltage)
{
	unsigned int current_voltage;

	if (voltage_checking(voltage) != 0)
		return 1;

#ifdef CONFIG_MX6_INTER_LDO_BYPASS
	// Shut off PU Power in analog
	reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) & (~0x00003E00)));
#else           
	current_voltage = reg_to_voltage((reg32_read(ANATOP_BASE_ADDR + 0x140) & (0x00003E00)) >> 9);

	// pu is power down at the initilization phase and keep power down
	// all the time, pu voltage change is only for leakage measurement purpose
	if ( (voltage == 0) & (current_voltage > 0) )
		// Shut off PU Power in analog
		reg32_write(ANATOP_BASE_ADDR + 0x140, reg32_read(ANATOP_BASE_ADDR + 0x140) & (~0x00003E00));
	else if ( (voltage > 0) & (current_voltage == 0) )
		// Turn on PU power in analog
		reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) & (~0x00003E00)) | (voltage_to_reg(voltage) << 9));
	else
		// voltage change
		reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) & (~0x00003E00)) | (voltage_to_reg(voltage) << 9));
#endif
    	
    return 0;
}

int system_config_soc_voltage(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_soc_voltage(unsigned int voltage)
{
	if (voltage_checking(voltage) != 0)
		return 1;
#ifdef CONFIG_MX6_INTER_LDO_BYPASS
		reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) | 0x007C0000));
#else
	reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) & (~0x007C0000)) | (voltage_to_reg(voltage) << 18));
#endif	

	return 0;
}

int system_config_arm_voltage(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_arm_voltage(unsigned int voltage)
{
	if (voltage_checking(voltage) != 0)
		return 1;
#ifdef CONFIG_MX6_INTER_LDO_BYPASS
	reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) | 0x0000001F));
#else	
	reg32_write(ANATOP_BASE_ADDR + 0x140, (reg32_read(ANATOP_BASE_ADDR + 0x140) & (~0x0000001F)) | (voltage_to_reg(voltage) << 0));
#endif	

	return 0;
}

int system_config_display_pg(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_display_pg(unsigned int enable)
{
	if (enable) {
		// Enable DISP Power Down
		reg32_write(GPC_BASE_ADDR + 0x240, 0x1);
		// Request DISP Power Down
		reg32_write(GPC_BASE_ADDR + GPC_CNTR_OFFSET, 0x10);
	} else
		// Request DISP Power Up
		reg32_write(GPC_BASE_ADDR + GPC_CNTR_OFFSET, 0x20);

	return 0;
}

int system_config_use_rc_osc (unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_use_rc_osc(unsigned int enable)
{
	unsigned int rc_osc_flag;

	rc_osc_flag = (reg32_read(ANATOP_BASE_ADDR + 0x260) & 0x00000010) >> 4;

	if (rc_osc_flag != enable) {
		// bypass all plls
		if (PLL_ARM_BYPASS == 0)
			system_config_arm_pll_bypass(1);

		if (PLL_528_BYPASS == 0)
			system_config_528_pll_bypass(1);

		if (PLL_480_BYPASS == 0)
			system_config_480_pll_bypass(1);

		if (enable)
			// switch from XTAL to RC
			reg32_write(ANATOP_BASE_ADDR + 0x260, reg32_read(ANATOP_BASE_ADDR + 0x260) | 0x00000010);
		else
			// switch from RC to XTAL
			reg32_write(ANATOP_BASE_ADDR + 0x260, reg32_read(ANATOP_BASE_ADDR + 0x260) & (~0x00000010));


		// enable plls again
		if (PLL_ARM_BYPASS == 0)
			system_config_arm_pll_bypass(0);

		if (PLL_528_BYPASS == 0)
			system_config_528_pll_bypass(0);

		if (PLL_480_BYPASS == 0)
			system_config_480_pll_bypass(0);
	}

	return 0;
}

int system_config_arm_pll_bypass (unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_arm_pll_bypass(unsigned int bypass)
{
	unsigned int bypass_flag;
	unsigned int power_down_flag;

	bypass_flag = (reg32_read(0x020C8000) & 0x00010000) >> 16;
	power_down_flag = (reg32_read(0x020C8000) & 0x00001000) >> 12;

	if ((bypass_flag == 0) && ((bypass == 1) || (bypass == 2)))
		reg32_write(0x020C8000, reg32_read(0x020C8000) | 0x00010000);


	if ((power_down_flag == 0) && (bypass == 2))
		reg32_write(0x020C8000, reg32_read(0x020C8000) | 0x00001000);

	if ((power_down_flag == 1) && ((bypass == 1) || (bypass == 0)))
		reg32_write(0x020C8000, reg32_read(0x020C8000) & (~0x00001000));

	if ((bypass_flag == 1) && (bypass == 0)) {
		while ( (reg32_read(0x020C8000) & 0x80000000) == 0x00000000);
		reg32_write(0x020C8000, reg32_read(0x020C8000) & (~0x00010000));
	}

	return 0;
}

int system_config_bus_clock_tree(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_bus_clock_tree(unsigned int bypass)
{
	if (bypass) {
		//disable handshake
		reg32_write(0x020c4004, reg32_read(0x020c4004) | 0x00020000);

		//select OSC as the source for periph_clk2_clk
		reg32_write(0x020C4018, (reg32_read(0x020C4018) & (~0x00003000)) | 0x00001000);

		//set periph_clk2_podf to 0
		reg32_write(0x020C4014, (reg32_read(0x020C4014) & (~0x38000000)) | 0x00000000);

		//select periph_clk2_clk as periph_clk
		reg32_write(0x020C4014, (reg32_read(0x020C4014) & (~0x02000000)) | 0x02000000);
	} else {
		//select pll2 as the source for pre_periph_clk
		reg32_write(0x020C4018, (reg32_read(0x020C4018) & (~0x000C0000)) | 0x00000000);

		//select pre_periph_clk as the source for periph_clk
		reg32_write(0x020C4014, (reg32_read(0x020C4014) & (~0x02000000)) | 0x00000000);
	}

	return 0;
}

int system_config_dram_clock_tree(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_dram_clock_tree(unsigned int bypass)
{
	if (bypass == 2) {
		//select pll3 as the source for periph2_clk2_clk
		reg32_write(0x020C4018, reg32_read(0x020C4018) & (~0x00100000) | 0x00000000);
		
		//set periph2_clk2_podf to 0
		reg32_write(0x020C4014, reg32_read(0x020C4014) & (~0x00000007) | 0x00000000);
		
		//select periph2_clk2_clk as periph2_clk
		reg32_write(0x020C4014, reg32_read(0x020C4014) & (~0x04000000) | 0x04000000);
	} else if (bypass == 1) {
		//select pll2 as the source for periph2_clk2_clk
		reg32_write(0x020C4018, (reg32_read(0x020C4018) & (~0x00100000)) | 0x00100000);
		
		//set periph2_clk2_podf to 0
		reg32_write(0x020C4014, (reg32_read(0x020C4014) & (~0x00000007)) | 0x00000000);

		//select periph2_clk2_clk as periph2_clk
		reg32_write(0x020C4014, (reg32_read(0x020C4014) & (~0x04000000)) | 0x04000000);
	} else {
		//select pfd400 as the source for pre_periph2_clk
		reg32_write(0x020C4018, (reg32_read(0x020C4018) & (~0x00600000)) | 0x00200000);

		//select periph2_clk_clk as periph2_clk
		reg32_write(0x020C4014, (reg32_read(0x020C4014) & (~0x04000000)) | 0x00000000);
	}

	return 0;
}

int system_config_bus_clock_divider(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_bus_clock_divider(unsigned int div)
{
	//set ocram(AXI) divider 
	reg32_write(0x020c4014, (reg32_read(0x020c4014) & (~0x00070000)) | ((div - 1) << 16));

	//set ahb divider
	reg32_write(0x020c4014, (reg32_read(0x020c4014) & (~0x00001C00)) | ((div - 1) << 10));

	return 0;
}

int system_config_dram_clock_divider(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_dram_clock_divider(unsigned int div)
{
	//set mmdc divider
	reg32_write(0x020c4014, (reg32_read(0x020c4014) & (~0x00000038)) | ((div-1) << 3));

	return 0;
}

int system_config_528_pll_bypass(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_528_pll_bypass(unsigned int bypass)
{
	unsigned int bypass_flag;
	unsigned int power_down_flag;

	bypass_flag = (reg32_read(0x020C8030) & 0x00010000) >> 16;
	power_down_flag = (reg32_read(0x020C8030) & 0x00001000) >> 12;

	if ((bypass_flag == 0) && ((bypass == 1) || (bypass == 2))) {
		system_config_bus_clock_tree(1);
		system_config_bus_clock_divider(1);

		system_config_dram_clock_tree(2);

		//bypass pll
		reg32_write(0x020C8030, reg32_read(0x020C8030) | 0x00010000);

		system_config_dram_clock_tree(1);
		system_config_dram_clock_divider(1);

		//shut off 400MPFD
		reg32_write(0x020C8100, reg32_read(0x020C8100) | 0x00800000);
	}

	if ((power_down_flag == 0) && (bypass == 2))
		//power down pll
		reg32_write(0x020C8030, reg32_read(0x020C8030) | 0x00001000);

	if ((power_down_flag == 1) && ((bypass == 1) || (bypass == 0)))
		//power up pll
		reg32_write(0x020C8030, reg32_read(0x020C8030) & (~0x00001000));


	if ((bypass_flag==1) && (bypass==0)) {
		//enable pll
		while ((reg32_read(0x020C8030) & 0x80000000) == 0x00000000);
		reg32_write(0x020C8030, reg32_read(0x020C8030) & (~0x00010000));

		//turn on PFDs
		reg32_write(0x020C8100, reg32_read(0x020C8100) & (~0x00800000));

		system_config_bus_clock_divider(4);
		system_config_dram_clock_divider(1);

		system_config_bus_clock_tree(0);
		system_config_dram_clock_tree(0);
	}

	return 0;
}

int system_config_480_pll_bypass(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_480_pll_bypass(unsigned int bypass)
{
	unsigned int bypass_flag;
	unsigned int power_down_flag;

	bypass_flag = (reg32_read(0x020C8010) & 0x00010000) >> 16;
	power_down_flag = 0x00000001 & (~((reg32_read(0x020C8010) & 0x00001000) >> 12));

	if ((bypass_flag == 0) && ((bypass == 1) || (bypass == 2)))
		reg32_write(0x020C8010, reg32_read(0x020C8010) | 0x00010000);

	if ((power_down_flag == 0) && (bypass == 2)) {
		//power down pll
		reg32_write(0x020C8010, reg32_read(0x020C8010) & (~0x00001000));
	}

	if ((power_down_flag == 1) && ((bypass == 1) || (bypass == 0))) {
		//power up pll
		reg32_write(0x020C8010, reg32_read(0x020C8010) | 0x00001000);
	}

	if ((bypass_flag == 1) && (bypass == 0)) {
		while ((reg32_read(0x020C8010) & 0x80000000) == 0x00000000);
		reg32_write(0x020C8010, reg32_read(0x020C8010) & (~0x00010000));
	}

	return 0;
}

int system_config_dram(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_dram(unsigned int enable)
{
	if (enable) {

		dram_data_init(DRAM_START_ADDR, 1000);
		if (dram_data_check(DRAM_START_ADDR, 1000)!=0)
			return 1;
		else
			return 0;
	} else
		return 1;
}

int system_config_arm_clock(unsigned int)
	__attribute__ ((section (".low_power_code")));
int system_config_arm_clock(unsigned int freq)
{
	if((freq == 800)||(freq == 1000)||(freq == 1200)) {
		// Bypass ARM PLL
		reg32_write(0x020C8000, reg32_read(0x020C8000) | 0x00010000);

		// Configure ARM PLL with new frequency
		reg32_write(0x020C8000, (reg32_read(0x020C8000) & (~0x0000003F)) | (0x3F & (freq*2/24)));
		while ((reg32_read(0x020C8000) & 0x80000000) == 0x00000000);

		 // Switch back to PLL clock output
		reg32_write(0x020C8000, reg32_read(0x020C8000) & (~0x00010000));
		return 0;
	} else
		return 1;
}

void system_config_init(void)
	__attribute__ ((section (".low_power_code")));
void system_config_init(void)
{
	system_config_480_pll_bypass(PLL_480_BYPASS);
	system_config_arm_pll_bypass(PLL_ARM_BYPASS);
	system_config_528_pll_bypass(PLL_528_BYPASS);

	if(PLL_528_BYPASS) {
		system_config_dram_clock_divider(2);
		system_config_bus_clock_tree(1);
		system_config_dram_clock_tree(1);
		system_config_bus_clock_divider(1);
	}

	system_config_arm_voltage(ARM_VOLTAGE);
	system_config_soc_voltage(SOC_VOLTAGE);
	system_config_pu_voltage(PU_VOLTAGE);

	system_config_display_pg(DISPLAY_PG);

	if(DRAM_EN)
		system_config_dram(1);
}

void system_config_enter_lpm(config_t * )
	__attribute__ ((section (".low_power_code")));
void system_config_enter_lpm(config_t * config)
{
	unsigned int lpm_parameter;

	if(config->parameter0 != config->parameter1) {
		lpm_parameter = config->parameter1;
		config->parameter1 = config->parameter0;
		update_config(config, lpm_parameter);
	}
}

void system_enter_lpm (void)
	__attribute__ ((section (".low_power_code")));
void system_enter_lpm(void)
{
	system_config_enter_lpm(system_config + SYS_CFG_ID_DRAM_EN);
	system_config_enter_lpm(system_config + SYS_CFG_ID_ARM_CLOCK_FREQ);
	system_config_enter_lpm(system_config + SYS_CFG_ID_BUS_CLOCK_FREQ);
	system_config_enter_lpm(system_config + SYS_CFG_ID_PLL_ARM_BYPASS);
	system_config_enter_lpm(system_config + SYS_CFG_ID_PLL_528_BYPASS);
	system_config_enter_lpm(system_config + SYS_CFG_ID_PLL_480_BYPASS);
	system_config_enter_lpm(system_config + SYS_CFG_ID_USE_RC_OSC);
	system_config_enter_lpm(system_config + SYS_CFG_ID_ARM_VOLTAGE);
	system_config_enter_lpm(system_config + SYS_CFG_ID_PU_VOLTAGE);
	system_config_enter_lpm(system_config + SYS_CFG_ID_SOC_VOLTAGE);
	system_config_enter_lpm(system_config + SYS_CFG_ID_DISPLAY_PG);
}
