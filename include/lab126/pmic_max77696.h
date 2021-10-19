/*
 * pmic_max77696.h 
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

#ifndef PMIC_MAX77696_H
#define PMIC_MAX77696_H

#define PMIC_I2C_ERROR       -1
#define PMIC_VNI_TEST_FAIL    0
#define PMIC_VNI_TEST_PASS    1
#define PMIC_CMD_FAIL    0
#define PMIC_CMD_PASS    1

#define LAB126_LOWBATT_VOLTAGE		3500

/* Functional block slave addresses */
#define FG_SADDR 0x34
#define USBIF_SADDR 0x35
#define PM_GPIO_SADDR 0x3C
#define RTC_SADDR 0x68

#define BAT_CAPACITY 0x0704
#define VF_FULLCAP 0x0704
#define MODEL_SCALING 0x1

#define MAX77696_CHARGER_CURRENT_TRICKLE 0x2
#define MAX77696_CHARGER_CURRENT_DEFAULT 0x9
#define MAX77696_CHARGER_CURRENT_FAST    0xe


/* GLBL_CNFG0 Register Values */
#define MAX77696_GLBL_CNFG0_PRSTRT   0x08
#define MAX77696_GLBL_CNFG0_SFTPDRR  0x01

#define BIT(x) 							(1<<(x))

#undef  BITS
#define BITS(_msb, _lsb)				((BIT(_msb)-BIT(_lsb))+BIT(_msb))


#define LAB126_LOWBATT_VOLTAGE		3500

#define CHGA_CHG_INT_OK_REG            0x0A
#define CHGA_CHG_INT_OK_CHGINA_M       BIT (6)
#define CHGA_CHG_CNFG_00_REG           0x0F	
#define CHGA_CHG_CNFG_00_MODE_M        BITS(3,0)
#define MAX77696_CHARGER_MODE_CHG     0b0101

void pmic_enable_usb_phy(int);

#endif /* PMIC_MAX77696_H */
