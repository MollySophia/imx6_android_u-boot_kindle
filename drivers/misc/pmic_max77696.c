/*
 * pmic_max77696.c 
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

/*!
 * @file pmic_max77696.c
 * @brief This file contains MAXIM 77696 specific PMIC code. This implementaion
 * may differ for each PMIC chip.
 *
 */

#include <common.h>
#include <lab126/pmic.h>
#include <lab126/pmic_max77696.h>
#include <linux/ctype.h>
#include <lab126/linux/boardid.h>
#include <lab126/linux/mfd/max77696_registers.h>
#include <lab126/linux/frontlight.h>

extern const u8 *get_board_id16(void);

#ifdef __DEBUG_PMIC__
#define DBG(fmt,args...)\
		serial_printf("[%s] %s:%d: "fmt,\
				__FILE__,__FUNCTION__,__LINE__, ##args)
#else
#define DBG(fmt,args...)
#endif

#define MAX77696_FG_OTP_TIMER_CHECK_DELAY       15*1000*1000  	/* in useconds */
#define MAX77696_FG_OTP_TIMER_ERROR_THRESHOLD   5

#define MAX77696_CHGCTRL_ENUM_EN  (1 << 3)
#define MAX77696_CHGCTRL_ENUM_SUB (1 << 6)
#define R_SNS 10000

#define MAX_BRIGHTNESS                      0xFFF /* 12bits resolution */
#define MAX_SCALED_BRIGHTNESS               255
#define MIN_BRIGHTNESS                      0

#define MID_VCOMR  -2499     //-2499mV
#define MAX_VCOMR  0         //0mV
#define MIN_VCOMR  -5000     //-5000mV
#define EPDVCOM_STEP 98


enum {
	EPDINTS_VC5FLTS_FAULT	,
	EPDINTS_VDDHFLTS_FAULT	,
	EPDINTS_VPOSFLTS_FAULT	,
	EPDINTS_VNEGFLTS_FAULT	,
	EPDINTS_VEEFLTS_FAULT	,
	EPDINTS_HVINPFLTS_FAULT	,
	EPDINTS_VCOMFLTS_FAULT	
};


#define __s16_to_intval(val) \
	(((val) & 0x8000)? -((int)((0x7fff & ~(val)) + 1)) : ((int)(val)))

#define PMIC_CHECK_POWER_GOOD_NUM_RETRY 	10

#ifdef CONFIG_MAX77696_FG_INIT

#define POR_RD_THRESHLD 5
#define CUSTOM_TBL_ROW 3
#define CUSTOM_TBL_COL 16

unsigned int custom_model[CUSTOM_TBL_ROW][CUSTOM_TBL_COL]=
	{
           {0xA8D0,0xB680,0xB960,0xBB80,0xBBE0,0xBC30,0xBD50,0xBE20,0xBE80,0xC090,0xC5A0,0xC730,0xC9D0,0xCC30,0xCE90,0xD110},
           {0x0160,0x0D30,0x0F30,0x3260,0x0CD0,0x1E90,0x2500,0x2FD0,0x11F0,0x0D80,0x0A40,0x0A70,0x09C0,0x09F0,0x04E0,0x04E0},
	   {0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100}
	};

static int pmic_fg_init(void);

#endif

int pmic_enable_epdc(int enable);
int pmic_check_power_good(void);
static int setup_epd_pmic(void);
static void setup_regulators(void);

int pmic_wor_reg(unsigned char saddr, unsigned int reg, const unsigned int value, const unsigned int mask)
{
    int ret;
    unsigned tmp;

    ret = pmic_read_reg(saddr, reg, &tmp);
    if (!ret) 
	return ret;
 
    tmp &= ~mask;
    tmp |= (value & mask);

    return pmic_write_reg(saddr, reg, tmp);
}

int pmic_check_factory_mode(void)
{
    printf("%s: begin\n", __func__);
    return 0;
}

void low_battery_check_loop(void)
{
	unsigned short batt_vol;
	int retry;
	if(PMIC_CMD_PASS == pmic_fg_read_voltage(&batt_vol)) {
		
		if(batt_vol < LAB126_LOWBATT_VOLTAGE) {
			printf("Low battery voltage=%dmV, please plug in charger and wait..\n", batt_vol);
			udelay(3 * 1000 *1000);
		}

		for(retry=10; 
			(batt_vol < LAB126_LOWBATT_VOLTAGE) && 
			(retry > 0); 
			/* no update */) {
			
			if( ! pmic_charging()) {
				retry--;
				printf("Not charging.. please connect to power source. time out in %d seconds\n", retry * 3);
			}
			
			if( PMIC_CMD_PASS == pmic_fg_read_voltage(&batt_vol) ) {
				// Loop until the battery charge to 
				printf("battery voltage=%dmV \n", batt_vol);				 
			}
			else{
				printf("%s:Battery voltage check failed ..\n", __func__);
				retry = 0;
				break;
			}
			udelay(3 * 1000 *1000);
			
		}//end for
		
		if(retry <= 0) {
			printf("Low battery and not charging.. system is going to halt .. \n");
			pmic_power_off();
		}
	}
	else {
		printf("%s:Battery voltage check failed ..\n", __func__);
		pmic_power_off();
	}
	return;
}

static void pmic_fl_init(void) 
{
	int i = 0; 
	int start_intensity = abc123_FL_LEVEL0, end_intensity = abc123_FL_LEVEL12_MID;
	int hop = abc123_FL_LO_GRP_HOP_LEVEL; 			/* FL intensity skip level */
	int delay_us = abc123_FL_LO_GRP_DELAY_US;		/* delay between FL intensity updates */

	for (i = start_intensity; i <= end_intensity; i += hop) {
		pmic_fl_set(i);  
		udelay(delay_us);
		if (i == abc123_FL_LO_TRANSITION_LEVEL) {
			hop = abc123_FL_MED_GRP_HOP_LEVEL;
		}
	}
}

static void setup_regulators(void)
{

	unsigned int cnfg=0x0, cid = 0x0;
	const char * rev = (const char *) get_board_id16();
	pmic_read_reg(PM_GPIO_SADDR, CID5_REG, &cid);
	/* bypass for rev.3+ PMICs */
	if(cid < 0x20) {
		/* Enable active discharge on all buck regulators
		Errata-RCT132: Will be fixed in REV-3 PMIC */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG1_REG, 0x50);
		pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG2_REG, 0x50);
		pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG3_REG, 0x50);
		pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG4_REG, 0x50);
		pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG5_REG, 0x50);
		pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG6_REG, 0x50);

		/* Update FPS configs on all buck regulators to add extra time for discharge
		Errata-RCT178: Will be fixed in REV-3 PMIC */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSBUCK1_REG, 0x9D);     /* BUCK1 from power-down time slot 3 to 5 */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSBUCK2_REG, 0x5D);     /* BUCK2 from power-down time slot 3 to 5 */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSBUCK3_REG, 0xA6);     /* BUCK3 from power-down time slot 4 to 6 */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSBUCK4_REG, 0x54);     /* BUCK4 from power-down time slot 2 to 4 */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSBUCK5_REG, 0x4B);     /* BUCK5 from power-down time slot 1 to 3 */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSBUCK6_REG, 0x42);     /* BUCK6 from power-down time slot 0 to 2 */
		pmic_write_reg(PM_GPIO_SADDR, BUCK_FPSRSTOUT_REG, 0x7F);    /* nRSTOUT from power-down time slot 5 to 7 */
	}

	/* No slew rate control */
	pmic_read_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG1_REG, &cnfg);
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG1_REG, (cnfg | 0xC0));
	pmic_read_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG2_REG, &cnfg);
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG2_REG, (cnfg | 0xC0));

	/* Set LDO 2 to the proper voltage for the USB PHY (3V) & enable it */
	pmic_write_reg(PM_GPIO_SADDR, LDO_L02_CNFG1_REG, 0xec);
	if (BOARD_REV_GREATER(rev, BOARD_ID_abc123_2)||
	    BOARD_IS_abc123(rev) ||
	    BOARD_IS_abc123(rev) ||
	    BOARD_IS_(rev, BOARD_ID_abc123_abc123_512, BOARD_ID_N) || 
	    BOARD_IS_(rev, BOARD_ID_abc123_WFO_abc123_512, BOARD_ID_N) ||
	    BOARD_REV_GREATER(rev, BOARD_ID_abc123_abc123_EVT1_2) ||
	    BOARD_REV_GREATER(rev, BOARD_ID_abc123_WFO_abc123_EVT1_2) ||
	    BOARD_REV_GREATER(rev, BOARD_ID_abc123_WFO_2GB_EVT1) ||
	    BOARD_REV_GREATER(rev, BOARD_ID_abc123_WFO_EVT1)) {
		// Bring LDO7 (keeper) up to 3.2 V, normal mode on boards with LDO7 Keeper
		pmic_write_reg(PM_GPIO_SADDR, LDO_L07_CNFG1_REG, 0x70);
	} else {
		//On boards without LDO7 keeper, just turn off LDO7
		pmic_write_reg(PM_GPIO_SADDR, LDO_L07_CNFG1_REG, 0x30);
	}

	if (BOARD_REV_GREATER(rev, BOARD_ID_abc123_EVT1_2) ||
		  BOARD_REV_GREATER(rev, BOARD_ID_abc123_2GB_EVT1_2) ||
		  BOARD_REV_GREATER(rev, BOARD_ID_abc123_WFO_2GB_EVT1_2) ||
	          BOARD_REV_GREATER(rev, BOARD_ID_abc123_WFO_EVT1_2) ||
                  BOARD_IS_abc123(rev)) {
		unsigned int old_val;
		pmic_read_reg(PM_GPIO_SADDR, LSW_SW3_CNTRL_REG, &old_val);
		pmic_write_reg(PM_GPIO_SADDR, LSW_SW3_CNTRL_REG, 
			 old_val & (~LSW_SW_CNTRL_LSADE_M));
	}

#ifdef CONFIG_MX6_INTER_LDO_BYPASS
	/* Set proper voltages for 1GHz operation */	
	/*Disable VDDPU : SW3, its unused */
	pmic_read_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG3_REG, &cnfg);
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUTCNFG3_REG, (cnfg & ~0x30));
	/*VDDSOC 1.2V : SW2 */
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUT2_REG, (unsigned int)0x30);
	/*VDDCORE 1.25V@1Ghz: SW1 */
	pmic_write_reg(PM_GPIO_SADDR, BUCK_VOUT1_REG, (unsigned int)0x34);

	/* Bypass internal LDOs */
	bypass_internal_pmic_ldo();
#endif
}

int pmic_enable_display_power_rail(unsigned int enable)
{
	int rval;

	rval = pmic_write_reg(PM_GPIO_SADDR, LSW_SW4_CNTRL_REG, enable);

	return rval;
}

int pmic_init(void) 
{
    int ret;
    const char * rev = (const char *) get_board_id16();
    ret = board_pmic_init();
   
    if (!ret)
	return ret;

    /* Force UIC to man ctrl that draws 0.5A from any charging src 
       to avoid drain-out scenario especially on dead battery boot-up 
       MANSET=b1 ; ISET=b011 (0.5A - SDP) */
    //pmic_write_reg(USBIF_SADDR, UIC_MANCTRL_REG, 0xB);

    /* abc123: Force UIC to man ctrl that draws 2.0A from any charging src
     * MANSET=b1 ; ISET=b110 (2.0A) */
    pmic_write_reg(USBIF_SADDR, UIC_MANCTRL_REG, 0xE);

    setup_regulators();

#ifdef CONFIG_GADGET_FASTBOOT
    /* BEN TODO - workaround for TO1.0 USB issue.  Make sure VBUS out is ~5V */
    pmic_write_reg(PM_GPIO_SADDR, LDO_L10_CNFG1_REG, 0xF4);

#endif
#ifdef CONFIG_MAX77696_FG_INIT
    pmic_fg_init();
#endif

    /* Enable power cycle feature after manual & wdog reset */
    pmic_write_reg(PM_GPIO_SADDR, GLBLCNFG2_REG, 0x36);

    ret = setup_epd_pmic();	
#ifndef CONFIG_MFGTOOL_MODE     	
    // loop until the battery charge to certain level or halt.
    low_battery_check_loop();
#endif //  CONFIG_MFGTOOL_MODE  

    /* Enable UIC auto enumeration only when system under reset (dead battery) 
		(i.e.) Disable UIC auto enumeration by default under normal scenario */
    pmic_wor_reg(USBIF_SADDR, UIC_CHGCTRL_REG, UIC_REG_BITSET(CHGCTRL, ENUMSUB, 1), UIC_CHGCTRL_ENUMSUB_M);

    /* Enable FL - early SOL */
    if (!BOARD_IS_abc123(rev)) 
        pmic_fl_init();

    return ret;
}

static int setup_epd_pmic(void)
{
    int ret = 1;
    
    /* Set default values for inrush, 
       will be fixed in Rev-3 PMIC */
    ret &= pmic_write_reg(PM_GPIO_SADDR, VREG_EPDCNFG_REG, 0x00);
    	
    ret &= pmic_write_reg(PM_GPIO_SADDR, VREG_EPDSEQ_REG, 0x18);
    ret &= pmic_write_reg(PM_GPIO_SADDR, VREG_EPDDIS_REG, 0xF2);

    if(!ret) {
        printf("%s: Setting default values failed!\n", __func__);
    }

    return ret;
}

void pmic_zforce2_pwrenable(int ena)
{
	/* turn LDO1 on to 3.2V to power zforce2 */
	if(ena)
		pmic_write_reg(PM_GPIO_SADDR, LDO_L01_CNFG1_REG, 0xF0);
	else
		pmic_write_reg(PM_GPIO_SADDR, LDO_L01_CNFG1_REG, 0x30);
}

int pmic_set_chg_current(pmic_charge_type chg_type)
{
    printf("%s: not implemented and return 1\n", __func__);
    return 1;
}
#define GLBL_CNFG0_FSENT	(1 << 5)
#define GLBL_CNFG0_SFTPDRR	(1 << 0)

int pmic_power_off(void)
{
	int ret = 1;
	
	ret &= pmic_write_reg(PM_GPIO_SADDR, GLBLCNFG0_REG, 0);
	ret &= pmic_write_reg(PM_GPIO_SADDR, GLBLCNFG0_REG, 
			GLBLCNFG0_FSENT | GLBLCNFG0_SFTPDRR);

	if (!ret) {
		printf("%s: Powering off device failed!\n", __func__);
	}

	return ret;
}

int pmic_reset(void)
{

    board_pmic_reset();

    return 1;
}

int pmic_charging(void)
{
	unsigned int status;
	unsigned int mode;

	if(!pmic_read_reg(PM_GPIO_SADDR, CHGA_CHG_INT_OK_REG, &status))
	{
		printf("PMIC read error (PM_GPIO_SADDR, CHGA_CHG_INT_OK_REG, &status) \n");
		return PMIC_I2C_ERROR;
	}
	if(!pmic_read_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, &mode))
	{
		printf("PMIC read error (PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, &mode) \n");
	    return PMIC_I2C_ERROR;
	}
	
	/*connected and enabled*/
	return (status & CHGA_CHG_INT_OK_CHGINA_M) && 
				((mode & CHGA_CHG_CNFG_00_MODE_M) == CHGA_CHG_CNFG_00_MODE_CHG);
	
}

int pmic_charge_restart(void)
{
    printf("%s: not implemented and return 1\n", __func__);
    return 1;
}

int pmic_set_autochg(int autochg)
{
    printf("%s: not implemented and return 1\n", __func__);
    return 1;
}

int pmic_start_charging(pmic_charge_type chg_type, int autochg)
{
    printf("%s: not implemented and return 1\n", __func__);
    return 1;
}

static unsigned short saved_result = BATTERY_INVALID_VOLTAGE;

int pmic_adc_read_last_voltage(unsigned short *result) {

	/* just return the last saved voltage result */
	if (saved_result == BATTERY_INVALID_VOLTAGE) {
		*result = 0;
		return 0;
	}

	*result = saved_result;
	return 1;
}

int pmic_adc_read_voltage(unsigned short *result)
{
	if( PMIC_CMD_PASS != pmic_fg_read_voltage(result) ) {
		printf("Cannot read battery voltage! \n");
		return PMIC_CMD_FAIL;
	}
	saved_result = *result;
	return PMIC_CMD_PASS;
}

int pmic_enable_green_led(int enable)
{
    printf("%s: begin\n", __func__);
    return board_enable_green_led(enable);
}

int pmic_set_alarm(unsigned int secs_from_now) 
{
    printf("%s: not implemented and return 1\n", __func__);
    return 1;
}

int pmic_rd_por(unsigned *porv)
{
   unsigned int status;
   pmic_read_reg(FG_SADDR, 0x00, &status);
   *porv = (status & 0x2); 

   return 0;
}

int pmic_wr_por(void)
{
  unsigned int status;
  pmic_read_reg(FG_SADDR, 0x00, &status);
  pmic_write_verify_reg(FG_SADDR, 0x00, (status & 0xFFFD));  

  return 0;
}

int pmic_charger_mode_set(int mode)
{
        unsigned int temp = 0;

  	if(!pmic_read_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, &temp)){
	    printf("PMIC read error (PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}
	mode |= (temp & 0xf0);
	if(!pmic_write_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, mode)){
	    printf("PMIC write error (PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, mode) \n");
	    return PMIC_I2C_ERROR;
	}
	return PMIC_CMD_PASS;
}

int pmic_charger_lock_config(int lock)
{
	unsigned int temp = 0;

        //Charger Settings Protection unlock
        if(!pmic_read_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_06_REG, &temp)){
	    printf("PMIC read error (PM_GPIO_SADDR, CHGA_CHG_CNFG_06_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}
	if(lock)
	{
	    temp = (temp & 0x3);
	}else{
	    temp = (temp & 0x3) | 0xc;
	}
	if(!pmic_write_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_06_REG, temp)){
	    printf("PMIC write error (PM_GPIO_SADDR, CHGA_CHG_CNFG_00_REG, mode) \n");
	    return PMIC_I2C_ERROR;
	}
	return PMIC_CMD_PASS;
}

int pmic_charger_set_current(int current)
{
	unsigned int temp = 0;

	//turn on charger
	pmic_charger_lock_config(0);
	pmic_charger_mode_set(CHGA_CHG_CNFG_00_MODE_CHG);    
  	if(!pmic_read_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_02_REG, &temp)){
	    printf("PMIC read error (PM_GPIO_SADDR, CHGA_CHG_CNFG_02_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}
	current |= (temp & 0xc0);	
	if(!pmic_write_reg(PM_GPIO_SADDR, CHGA_CHG_CNFG_02_REG, current)){
	    printf("PMIC write error (PM_GPIO_SADDR, CHGA_CHG_CNFG_02_REG, current) \n");
	    return PMIC_I2C_ERROR;
	}
	pmic_charger_lock_config(1);
	return PMIC_CMD_PASS;
}

int pmic_fg_read_voltage(unsigned short *voltage)
{
	unsigned int temp =0;	
	if(!pmic_read_reg(FG_SADDR, FG_VCELL_REG, &temp)){
	    printf("PMIC read error (FG_SADDR, FG_VCELL_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}else {	
	   *voltage = (unsigned short)((((temp & 0xffff) >> 3) * 625) / 1000);            
	}
	return PMIC_CMD_PASS;
}

int pmic_fg_read_avg_current(int *curr)
{
	unsigned int temp =0;	
	int cur_val;

	if(!pmic_read_reg(FG_SADDR, FG_AVGCURRENT_REG, &temp)){
	    printf("PMIC read error (FG_SADDR, FG_AVGCURRENT_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}
         
	cur_val = __s16_to_intval(temp);
	/* current in uA ; r_sns = 10000 uOhms */
	cur_val *= (1562500 / R_SNS);
	/* current in mA */
	*curr = cur_val / 1000;           
	
    return PMIC_CMD_PASS;
}


int pmic_fg_read_capacity(unsigned short *capacity)
{
	unsigned int temp =0;	
	if(!pmic_read_reg(FG_SADDR, FG_SOCREP_REG, &temp)){
	    printf("PMIC read error (FG_SADDR, FG_SOCREP_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}else {
            *capacity = (unsigned short)((temp & 0xffff) >> 8);
    }
    return PMIC_CMD_PASS;
}

int pmic_fg_read_temperature(int *temperature)
{
	unsigned int temp = 0;	
	int temp_val;

	if(!pmic_read_reg(FG_SADDR, FG_TEMP_REG, &temp)){
	    printf("PMIC read error (FG_SADDR, FG_TEMP_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
	}
	
	temp_val =  __s16_to_intval(temp);
	/* Ignore LSB for 1 deg celsius resolution */
	*temperature = (temp_val >> 8);           
	
    return PMIC_CMD_PASS;
}

int pmic_fg_otp_check(void)
{
    unsigned int temp0 = 0, temp1 = 0;;	
    unsigned int fg_timer0 =0 , fg_timer1= 0;
    unsigned int rtc_timer0 =0 , rtc_timer1= 0;
    unsigned int fg_delta = 0, rtc_delta = 0, delta_comp = 0;
    
    if(!pmic_read_reg(FG_SADDR, FG_TIMER_REG, &temp0)){
        printf("PMIC read error (FG_SADDR, FG_TEMP_REG, &temp0) \n");
        return PMIC_I2C_ERROR;
    }
    fg_timer0 = (temp0 * 1758) / 10000;	/* seconds */
    printf("fg_timer0 = %d \n", fg_timer0); 
    
    temp0 = 0x18;
    pmic_write_reg(RTC_SADDR, RTC_RTCUPDATE0_REG, temp0);
    
    if(!pmic_read_reg(RTC_SADDR, RTC_RTCSEC_REG, &temp0)){
        printf("PMIC read error (RTC_SADDR, RTC_RTCSEC_REG, &temp0) \n");
        return PMIC_I2C_ERROR;
    }
    rtc_timer0 = temp0 & 0x3F;	/* seconds */
    printf("rtc_timer0 = %d \n", rtc_timer0); 
    
    udelay(MAX77696_FG_OTP_TIMER_CHECK_DELAY);
    
    if(!pmic_read_reg(FG_SADDR, FG_TIMER_REG, &temp1)){
        printf("PMIC read error (FG_SADDR, FG_TEMP_REG, &temp1) \n");
        return PMIC_I2C_ERROR;
    }
    fg_timer1 = (temp1 * 1758) / 10000;	/* seconds */
    printf("fg_timer1 = %d \n", fg_timer1); 
    
    temp1 = 0x18;
    pmic_write_reg(RTC_SADDR, RTC_RTCUPDATE0_REG, temp1);
    
    if(!pmic_read_reg(RTC_SADDR, RTC_RTCSEC_REG, &temp1)){
        printf("PMIC read error (RTC_SADDR, RTC_RTCSEC_REG, &temp1) \n");
        return PMIC_I2C_ERROR;
    }
    rtc_timer1 = temp1 & 0x3F;	/* seconds */
    printf("rtc_timer1 = %d \n", rtc_timer1); 
    
    fg_delta = fg_timer1 - fg_timer0;
    rtc_delta = ((rtc_timer1 > rtc_timer0) ? (rtc_timer1 - rtc_timer0) : (60 - (rtc_timer0 - rtc_timer1))); 
    
    printf("FG(delta) = %d, RTC(delta) = %d \n", fg_delta, rtc_delta);
    delta_comp = ((rtc_delta > fg_delta) ? (rtc_delta - fg_delta) : (fg_delta - rtc_delta));
    
    if ( delta_comp > MAX77696_FG_OTP_TIMER_ERROR_THRESHOLD ) 
    	printf("fg otp data check FAIL \n");
    else 
    	printf("fg otp data check PASS \n");
    
    return PMIC_CMD_PASS;
}

int pmic_fl_enable ( int enable)
{
    unsigned int temp = 0;

    if(!pmic_read_reg(PM_GPIO_SADDR, WLED_LEDBST_CNTRL1_REG, &temp)){
	    printf("PMIC read error (PM_GPIO_SADDR, WLED_LEDBST_CNTRL1_REG, &temp) \n");
	    return PMIC_I2C_ERROR;
    }
    temp = enable ? (temp | 0x80): (temp & 0x7f);

    if(!pmic_write_reg(PM_GPIO_SADDR, WLED_LEDBST_CNTRL1_REG, temp)){
	    printf("PMIC write error (PM_GPIO_SADDR, WLED_LEDBST_CNTRL1_REG, temp) \n");
	    return PMIC_I2C_ERROR;
    }   
    return PMIC_CMD_PASS;
}

int pmic_fl_set (int brightness)
{
    u8 buf[2];
    const char * rev = (const char *) get_board_id16();
    if (brightness < MIN_BRIGHTNESS || brightness > MAX_BRIGHTNESS) {
          pmic_fl_enable(0);
          return PMIC_VNI_TEST_FAIL;
    }

    /* LED1CURRENT_1 (MSB: BIT11 ~ BIT4), LED1CURRENT_2 (LSB: BIT3 ~ BIT0) */

    buf[0] = (u8)(brightness >> 4);
    buf[1] = (u8)(brightness & 0xf);
	
    if(!pmic_write_reg(PM_GPIO_SADDR, WLED_LED1CURRENT_1_REG, buf[0])){
	  printf("PMIC write error (PM_GPIO_SADDR, WLED_LED1CURRENT_1_REG, buf[1]) \n");
	  return PMIC_I2C_ERROR;
    }   
    if(!pmic_write_reg(PM_GPIO_SADDR, WLED_LED1CURRENT_2_REG, buf[1])){
	  printf("PMIC write error (PM_GPIO_SADDR, WLED_LED1CURRENT_2_REG, buf[0]) \n");
	  return PMIC_I2C_ERROR;
    }
    if (!BOARD_IS_abc123(rev)) 
        if(pmic_fl_enable(1) != PMIC_CMD_PASS){
            printf("failed to be enabled blacklight\n");
            return PMIC_I2C_ERROR;
        }

    return PMIC_CMD_PASS;
}
 

int check_EPDINTS_reg(void)
{
	unsigned int temp,val = 0, n = 0;
	
	if(!pmic_read_reg(PM_GPIO_SADDR,VREG_EPDINTS_REG,&val))
	{
		printf("Read VREG_EPDINTS_REG register failed\n");
		return PMIC_I2C_ERROR;
	}
	if(val == 0)
	{
		printf("EPDINTS No fault!\n");
		return PMIC_CMD_PASS;
	}
	else
	{
		//printf("val=%x\n",val);
		while( n < 8)
		{		

			temp = val >> n;
			//printf("temp = 0x%x, n=%d\n",temp,n);
			if(temp & 0x1)
			{
				switch( n )
				{
					case EPDINTS_VC5FLTS_FAULT:
						printf("VC5FLTS_FAULT\n");
						break;
					case EPDINTS_VDDHFLTS_FAULT:
						printf("VDDHFLTS_FAULT\n");
						break;
					case EPDINTS_VPOSFLTS_FAULT:
						printf("VPOSFLTS_FAULT\n");
						break;
					case EPDINTS_VNEGFLTS_FAULT:
						printf("VNEGFLTS_FAULT\n");
						break;
					case EPDINTS_VEEFLTS_FAULT:
						printf("VEEFLTS_FAULT\n");
						break;
					case EPDINTS_HVINPFLTS_FAULT:
						printf("HVINPFLTS_FAULT\n");
						break;
					case EPDINTS_VCOMFLTS_FAULT:
						printf("VCOMFLTS_FAULT\n");
						break;
					default:
						break;						
				}
			}
			n++;
		}
		return PMIC_VNI_TEST_FAIL;
	}

}

int is_epdc_on(void)
{
    unsigned int val = 0;
    if(!pmic_read_reg(PM_GPIO_SADDR,VREG_EPDCNFG_REG,&val))
    {
	    printf("PMIC read error (PM_GPIO_SADDR,VREG_EPDCNFG_REG,&val)\n");
	    return PMIC_I2C_ERROR;
    }
	return (val & VREG_EPDOKINT_EPDPOK_M)? 1 : 0;	
}

int set_vcom_value(char* value)
{
	 
	int vcom_val = 0;
        int epdvcom = 0;
	
	printf(" set Vcom value:%smV\n",value);	
	
	vcom_val = simple_strtol(value,NULL,10);
	
	if(vcom_val < MIN_VCOMR || vcom_val > MAX_VCOMR)
	{
             printf("Valid Vcom range is 0 to -5000mV\n");
             return PMIC_VNI_TEST_FAIL;
	}
	
	if(vcom_val < MID_VCOMR)
	{
       	    if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDVCOMR_REG,0x1))
	    {
		    printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDVCOMR_REG,0x1)\n");
		    return PMIC_I2C_ERROR;
	    }
	    vcom_val = vcom_val - MID_VCOMR;
	}else{
       	    if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDVCOMR_REG,0x0))
	    {
		    printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDVCOMR_REG,0x1) \n");
		    return PMIC_I2C_ERROR;
	    }

	}
	epdvcom = (int)((vcom_val * -10)/EPDVCOM_STEP);
   
	if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDVCOM_REG,epdvcom))
	{
		   printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDVCOMR_REG,epdvcom) \n");
		   return PMIC_I2C_ERROR;
	} 	
	return PMIC_CMD_PASS;
}

int pmic_vcom_set(char* vcom_val)
{
	int retry = PMIC_CHECK_POWER_GOOD_NUM_RETRY;
	if (!is_epdc_on())
	{
		do {
			if (pmic_enable_epdc(1) == PMIC_CMD_PASS)
				break;
			/* if it cannot enable, turn off, then on again */
			printf("%s pmic_enable_epdc(1) failed, retry = %d\n", __func__, retry);
			pmic_enable_epdc(0);
			/* minimum of 16 ms delay needed before turning it on */
			udelay(20000);
		} while (--retry > 0);
	}
	if (!is_epdc_on()) {
		return PMIC_VNI_TEST_FAIL;
	}
      /*
       sets VCOM value
      */
     if(set_vcom_value(vcom_val) != PMIC_CMD_PASS)
     {
	    pmic_enable_epdc(0);
		return PMIC_VNI_TEST_FAIL;
     }
     pmic_enable_vcom(1); 
     return PMIC_CMD_PASS;
}

int pmic_check_power_good(void)
{
    unsigned int val;

    if (!pmic_read_reg(PM_GPIO_SADDR, VREG_EPDOKINTS_REG, &val))
    {
        printf("Read EPDIKINTS register failed\n");
        return PMIC_I2C_ERROR;
    }
    if (val & VREG_EPDOKINT_EPDPOK_M)
    {
        printf("%s EPD Power-Ok PASS [0x%04x]\n", __func__, val);
        return PMIC_CMD_PASS;
    }
    printf("%s EPD Power-Ok FAIL [0x%04x]\n", __func__, val);
    return PMIC_CMD_FAIL;
}

int pmic_enable_vcom(int enable)
{
    int ret;
    int retry = PMIC_CHECK_POWER_GOOD_NUM_RETRY;

    if(enable){	
        /*
	 sets VCOM = 1, leaves EPDEN = 1 as set in
         step 1, and leaves soft-start ramp rates for each of the EPD regulators at their default setting
        */
        do {
            ret = pmic_check_power_good();
            if (ret == PMIC_CMD_PASS) {
                break;;
            }
            udelay(100);
        } while (--retry > 0);
        if (retry == 0) {
            printf("%s EPD Power-Ok FAIL\n", __func__);
            return PMIC_CMD_FAIL;
        } 
        if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDCNFG_REG,0xfe)){
	    printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDCNFG_REG,0xfe) \n");
            return PMIC_I2C_ERROR;
        }
     }else{			
	/*
	Write 0xBEh to register 0x60h on slave address 0x3c
	sets VCOMEN = 0, leaves EPDEN = 1 (assuming that the supplies are already enabled), 
	and leaves soft-start ramp rates for each of the EPD regulators at their default setting
       */
	     if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDCNFG_REG,0xbe)){
		     printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDCNFG_REG,0xbe) \n");
		     return PMIC_I2C_ERROR;
	     }
             pmic_enable_epdc(0);
    }
	    return PMIC_CMD_PASS;
}

int pmic_enable_epdc(int enable)
{
	unsigned int val = 0;
	int count = 0;

	if(enable){
		/*
		Write 0xBEh to register 0x60h on slave address 0x78h (sets EPDEN = 1, and leaves soft-start
		ramp rates for each of the EPD regulators at their default setting) 
		*/
             if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDCNFG_REG,0xbe))
             {
                 printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDCNFG_REG,0xbe) \n");
                 return PMIC_I2C_ERROR;
             }
             /*
	      check EPDINTS register to see if there is any rail fault
	     */
            if(check_EPDINTS_reg() !=PMIC_CMD_PASS ){
		 printf("EPDINTS faults\n");
		 return PMIC_VNI_TEST_FAIL;
            }			 
	    /*
	     Wait for indication that EPD supplies are regulating
	    */
	    do{
		if(!pmic_read_reg(PM_GPIO_SADDR,VREG_EPDOKINTS_REG,&val))
		{
			printf("PMIC read error (PM_GPIO_SADDR,VREG_EPDOKINTS_REG,&val) \n");
			return PMIC_I2C_ERROR;
		}
		if (val & VREG_EPDOKINT_EPDPOK_M)
			return PMIC_CMD_PASS;
		udelay(20000);
		count++;
	    } while(count <= 10);
		/* return error if the power ok is not good */
		return PMIC_VNI_TEST_FAIL;
	} else {
	   /*
	    Write 0x3F to register 0x60h on slave address 0x3ch (sets EPDEN = 0, leaves EPDEN = 0, and
	    leaves soft-start ramp rates for each of the EPD regulators at their default setting)
	   */
           if(!pmic_write_reg(PM_GPIO_SADDR,VREG_EPDCNFG_REG,0x3f))
           {
                 printf("PMIC write error (PM_GPIO_SADDR,VREG_EPDCNFG_REG,0x3f) \n");
                 return PMIC_I2C_ERROR;
            }
   }
   return PMIC_CMD_PASS;
}

void pmic_enable_usb_phy(int enable) {
    if (enable) {
        pmic_write_reg(PM_GPIO_SADDR, LDO_L02_CNFG1_REG, 0xec);
    } else {
        pmic_write_reg(PM_GPIO_SADDR, LDO_L02_CNFG1_REG, 0x2c);
    }
}

#ifdef CONFIG_MAX77696_FG_INIT
/* Fuel gauge initial settings */
static int pmic_fg_init(void)
{
    unsigned por=0x0;

    pmic_rd_por(&por);

    if(por) {
		/* After Power up, the MAX77696 Fuel Gauge requires 500mS in order
		 * to perform signal debouncing, OCV measurement & initial SOC reporting 
		 */
		udelay(500*1000);
	}

   return 0;
}

#if 0
static int pmic_fg_init_config(void)
{
   unsigned int vfsoc, rem_cap, rep_cap, dQ_acc;

   // 2. Initialize Configuration
   pmic_write_reg(FG_SADDR, 0x1D, 0x2210); // CONFIG
   pmic_write_reg(FG_SADDR, 0x29, 0x87A4); // FilterCFG
   // remain the default value of RelaxCFG until Standby current is known.
   // pmic_write_reg(0x2A, RelxCFG_Value) // RelaxCFG
   pmic_write_reg(FG_SADDR, 0x28, 0x2606); // LearnCFG
   pmic_write_reg(FG_SADDR, 0x13, 0x5A00); // FullSOCthr=95%

   // Load Custom Model
   pmic_fg_load_custom_model();

   // 10. Write Custom Parameters
   pmic_write_verify_reg(FG_SADDR, 0x38, 0x0041); // RCOMP0
   pmic_write_verify_reg(FG_SADDR, 0x39, 0x1121); // TempCo
   pmic_write_reg(FG_SADDR, 0x1E, 0x0100); // ICHGTerm
   // Do not set the TGAIN and TOFF, because this board didn't use thermistor.
   // pmic_write_reg(0x2C, TGAIN)
   // pmic_write_reg(0x2D, TOFF)
   pmic_write_verify_reg(FG_SADDR, 0x3A, 0xACDA); // V_Empty
   pmic_write_verify_reg(FG_SADDR, 0x12, 0x2186); // QRTable00
   pmic_write_verify_reg(FG_SADDR, 0x22, 0x2186); // QRTable10
   pmic_write_verify_reg(FG_SADDR, 0x32, 0x0680); // QRTable20
   pmic_write_verify_reg(FG_SADDR, 0x42, 0x0501); // QRTable30

   // 11. Update Full Capacity Parameters
   pmic_write_verify_reg(FG_SADDR, 0x10, VF_FULLCAP); // FullCap
   pmic_write_reg(FG_SADDR, 0x18, VF_FULLCAP /*VF_FullCap*/); // write designCap
   pmic_write_verify_reg(FG_SADDR, 0x23, VF_FULLCAP /*VF_FullCap*/); // FullCapNom

   // 13. Delay 350ms
   udelay(350*1000);

   // 14. Write VFSOC value to VFSOC0
   pmic_read_reg(FG_SADDR, 0xFF, &vfsoc); // VFSOC
   pmic_write_verify_reg(FG_SADDR, 0x60, 0x0080); // Enable Write Access to VFSOC0
   pmic_write_reg(FG_SADDR, 0x48, vfsoc); // write vfsoc to VFSOC0
   pmic_write_verify_reg(FG_SADDR, 0x60, 0x0000); // Disable Write Access to VFSOC0

   // 15.5 Advance to Coulomb-Counter Mode
   pmic_write_verify_reg(FG_SADDR, 0x17, 0x0060); //96% for Max17047, Cycles

   // 16. Load New Capacity Parameters
   rem_cap = (vfsoc * VF_FULLCAP)/25600;
   pmic_write_reg(FG_SADDR, 0x0f, rem_cap);

   rep_cap = rem_cap * (BAT_CAPACITY/VF_FULLCAP) / MODEL_SCALING;
   pmic_write_reg(FG_SADDR, 0x05, rep_cap);

   // Write dQ_acc to 200% of Capacity and dP_acc to 200%
   dQ_acc = (VF_FULLCAP/4);
   pmic_write_verify_reg(FG_SADDR, 0x45, dQ_acc);
   pmic_write_verify_reg(FG_SADDR, 0x46, 0x3200); // dP_acc
   pmic_write_verify_reg(FG_SADDR, 0x10, BAT_CAPACITY); // FullCap
   pmic_write_reg(FG_SADDR, 0x18, VF_FULLCAP); // DesignCap
   pmic_write_verify_reg(FG_SADDR, 0x23, VF_FULLCAP); // FullCapNom
   // Update SOC register with new SOC
   pmic_write_reg(FG_SADDR, 0x06, vfsoc); // SOCRep
   
   return 0;
}

static int pmic_fg_load_custom_model(void)
{
   unsigned int compare_data[CUSTOM_TBL_ROW][CUSTOM_TBL_COL];
   unsigned char i,j;

   // 4. Unlock Model Access
   pmic_write_verify_reg(FG_SADDR, 0x62, 0x0059);
   pmic_write_verify_reg(FG_SADDR, 0x63, 0x00C4);

   // 5. Write/Read/Verify the Custom Model
   for(i=0;i<CUSTOM_TBL_COL;i++) 
     pmic_write_verify_reg(FG_SADDR, (0x80+i), custom_model[0][i]);
   for(i=0;i<CUSTOM_TBL_COL;i++) 
     pmic_write_verify_reg(FG_SADDR, (0x90+i), custom_model[1][i]);
   for(i=0;i<CUSTOM_TBL_COL;i++) 
     pmic_write_verify_reg(FG_SADDR, (0xA0+i), custom_model[2][i]);
   
   // 8. Lock Model Access
   pmic_write_verify_reg(FG_SADDR, 0x62, 0x0000);
   pmic_write_verify_reg(FG_SADDR, 0x63, 0x0000);

   for(i=0;i<CUSTOM_TBL_ROW;i++)
   {
      for(j=0;j<CUSTOM_TBL_COL;j++)
      {
         compare_data[i][j] = 0x0;
         pmic_read_reg(FG_SADDR, (0x80+i), &compare_data[i][j]);
         if(compare_data[i][j] != 0x0)
         {
            DBG("\n%s, Lock Model access failed!\n", __func__);
         }
      }
   }

   return 0;
}
#endif

#endif
