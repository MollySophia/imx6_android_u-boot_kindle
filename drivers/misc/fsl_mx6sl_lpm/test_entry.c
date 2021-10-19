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

#include "common.h"
#include "dram_api.h"
#include "config.h"
#include "low_power_mode.h"
#include "system_config.h"
#include <i2c.h>
//#include "asm-arm/arch-mx6/regs-anadig.h"
#include <fsl-mx6sl-lpm.h>
#include <boardid.h>

#ifdef CONFIG_PMIC
#include <lab126/pmic.h>
#endif
#ifdef CONFIG_PMIC_ROHM
#include <lab126/pmic_rohm.h>
#endif
#ifdef CONFIG_PMIC_MAX77696
#include <lab126/pmic_max77696.h>
#include <lab126/linux/mfd/max77696_registers.h>
extern int pmic_fl_enable ( int enable);
#endif

char __low_power_code_start[0] __attribute__((section(".__low_power_code_start")));
char __low_power_code_end[0] __attribute__((section(".__low_power_code_end")));
void init_low_power_code(void)
{
	memcpy((char*)CONFIG_FSL_MX6SL_LPM_VMA, __low_power_code_start, (__low_power_code_end - __low_power_code_start));
}

extern config_t lpm_config[];
extern config_t system_config[];
//extern void lpm_suspend_iomux_setup(void);
//extern void reset_touch_zforce2_pins(void);
extern const u8 *get_board_id16(void);
unsigned long current_lpm_config /*__attribute__ ((section (".bss_low_power")))*/ = 0;

static void i2c_clock_enable(void)
{
	reg32_write(CCM_CCGR2, reg32_read(CCM_CCGR2) | 0x000000C0);
}

static void i2c_clock_disable(void)
{
	reg32_write(CCM_CCGR2, reg32_read(CCM_CCGR2) & (~0x000000C0));
}

void lower_arm_soc_voltages(void)
{
	// Set SoC LDO to Bypass
	update_config(system_config + SYS_CFG_ID_SOC_VOLTAGE     , 1500);
	//Lower ARM frequency
	update_config(system_config + SYS_CFG_ID_PLL_ARM_BYPASS     , 1);

	i2c_clock_enable();

	//Lower ARM & SoC voltages
#ifdef CONFIG_PMIC_MAX77696
	/*VDDCORE 0.95V: SW1*/
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUT1_REG, (unsigned int)0x1C);
	/*VDDSOC 1.15V : SW2*/
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUT2_REG, (unsigned int)0x2C);
#endif
#ifdef CONFIG_PMIC_ROHM
	pmic_write_reg(ROHM_I2C_ADDR, BD7181X_REG_BUCK1_VOLT_H, (unsigned int)0xcc);
	pmic_write_reg(ROHM_I2C_ADDR, BD7181X_REG_BUCK2_VOLT_H, (unsigned int)0x84);
#endif
	
	i2c_clock_disable();
}

extern void board_entry_low_power_mode(void);
extern void init_low_power_code(void);
extern void (*mx6q_suspend_t)(unsigned int mode, unsigned int addr0, unsigned int addr1);

// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int test_entry (unsigned long)
    __attribute__ ((section (".low_power_code")));
int test_entry(unsigned long test_num)
{
	unsigned long action_id;
#ifdef CONFIG_PMIC_MAX77696
	unsigned int pmic_cnfg = 0x0;
	unsigned int pmic_glblcnfg0 = 0;
#endif
#ifdef CONFIG_PMIC_ROHM
	unsigned int pmic_pwrctrl = 0;
#endif
    const char * rev = (const char *) get_board_id16();
	asm ("ldr sp, =0x00960000"); // Change the stack pointer to IRAM

	action_id = test_num;

	init_low_power_code();
	printf("copy low power code done %x\n",*(u32 *)0x920000);

	/* Mask MMDC channel 0 Handshake */
	reg32_write(0x020c4004, 0x20000);

#ifdef CONFIG_PMIC_MAX77696
	/* disable FL */
        if (!BOARD_IS_abc123(rev)) 
            pmic_fl_enable(0);
#endif

	if(action_id == TEST_ID_SUSPEND_MODE) {	
#ifdef CONFIG_PMIC_MAX77696
		// set STBYEN bit
		pmic_read_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG1_REG, &pmic_cnfg);
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG1_REG, 
			(unsigned int)(pmic_cnfg| 1<<GLBLCNFG1_STBYEN_SHIFT));

		// adjust LDO7 voltage down to 1.8 - maintain previous enabled/disabled state
		if (!BOARD_IS_abc123(rev)) {
			pmic_read_reg(PM_GPIO_SADDR, (unsigned int)LDO_L07_CNFG1_REG, &pmic_cnfg);
			pmic_write_reg(PM_GPIO_SADDR, (unsigned int)LDO_L07_CNFG1_REG,
				(unsigned int)( (pmic_cnfg & LDO_CNFG1_PWRMD_M) | 0x14) );
		}

		if ( BOARD_IS_abc123(rev) || BOARD_IS_abc123_4_256M_CFG_C(rev)
			|| BOARD_IS_abc123_PREEVT2(rev) ) {
#ifdef DEVELOPMENT_MODE
			printf("turning-off LDO touch power in suspend..");
#endif
//			reset_touch_zforce2_pins();
			/* delay for pin configs to take effect */
			udelay(100);
			pmic_zforce2_pwrenable(0);
#ifdef CONFIG_PMIC_ROHM
			pmic_write_reg(ROHM_I2C_ADDR,BD7181X_REG_LDO_MODE3,0x00);
#endif	
			
		}
#endif
#ifdef CONFIG_PMIC_ROHM
		// Send SLEEP NOTIFICATION before cutting off VCC
		{
			extern void mmc_suspend();
			mmc_suspend();
		}
    pmic_write_reg(ROHM_I2C_ADDR, BD7181X_REG_BUCK1_VOLT_L, (unsigned int)0x4);
	pmic_write_reg(ROHM_I2C_ADDR, BD7181X_REG_BUCK5_MODE, (unsigned int)0x4);
#endif
	}

	if(action_id == TEST_ID_SHIPPING_MODE) {
#ifdef CONFIG_PMIC_MAX77696
		pmic_read_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG0_REG, &pmic_glblcnfg0);
		pmic_write_reg(PM_GPIO_SADDR, (unsigned int)GLBLCNFG0_REG,
			(unsigned int)(pmic_glblcnfg0 | GLBLCNFG0_FSENT));
#endif
#ifdef CONFIG_PMIC_ROHM
		pmic_read_reg(ROHM_I2C_ADDR,(unsigned int)BD7181X_REG_PWRCTRL, &pmic_pwrctrl);
		pmic_write_reg(ROHM_I2C_ADDR,(unsigned int)BD7181X_REG_PWRCTRL, (pmic_pwrctrl | 1));
#endif
	}

	if(action_id == TEST_ID_IDLE_MODE) {
#ifdef CONFIG_PMIC_ROHM
			pmic_write_reg(ROHM_I2C_ADDR,BD7181X_REG_LDO_MODE3,0x04);
#endif
	}

	board_entry_low_power_mode();

#ifdef CONFIG_PMIC_MAX77696
	pmic_write_reg(PM_GPIO_SADDR, (unsigned int)LDO_L02_CNFG1_REG,0);
	pmic_write_reg(PM_GPIO_SADDR, (unsigned int)LDO_L02_CNFG1_REG,0);
	pmic_write_reg(PM_GPIO_SADDR, (unsigned int)BUCK_VOUTCNFG6_REG,0);
#endif

	/* Need to mask the Brown out interrupt as PU is disabled. */
	reg32_write(GPC_BASE_ADDR + GPC_IMR3_OFFSET, 0x20000000); //95:64
	reg32_write(GPC_BASE_ADDR + GPC_IMR4_OFFSET, 0x80000000); //127:96

	reg32_write(ANATOP_BASE_ADDR + 0x150, reg32_read(ANATOP_BASE_ADDR + 0x150) | 0x00002000);

	switch(action_id)
	{
		case TEST_ID_SUSPEND_MODE:
			current_lpm_config = LPM_LAB126_SUSPEND_MODE;
			reg32_write(ANATOP_BASE_ADDR + 0x130, reg32_read(ANATOP_BASE_ADDR + 0x130) | 0x00000008);
			lpm_config_mode(LPM_LAB126_SUSPEND_MODE);
			lpm_config_init();
			system_config_init();
			lpm_enter_exit();
			reg32_write(ANATOP_BASE_ADDR + 0x130, reg32_read(ANATOP_BASE_ADDR + 0x130) & (~0x00000008));
			break;
		case TEST_ID_IDLE_MODE:
			current_lpm_config = LPM_LAB126_IDLE_2_MODE;
			/* 3v2 display power gate will be enabled in idle */
#ifdef CONFIG_PMIC_MAX77696
			pmic_enable_display_power_rail(1); 
#endif
			/* Idle without ARM powergate mode */
			lpm_config_mode(LPM_LAB126_IDLE_2_MODE); 
			lpm_config_init();
			system_config_init();
			lower_arm_soc_voltages();
			lpm_enter_exit();
			break;
		default:
			break;
	}

	return 0;
}
