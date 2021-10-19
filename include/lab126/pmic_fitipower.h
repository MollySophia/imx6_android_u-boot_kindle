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

#ifndef PMIC_FITIPOWER_H
#define PMIC_FITIPOWER_H

#define FITIPOWER_I2C_ADDR 0x48
#define TMST_VALUE_REG 0x0
#define VCOM_SETTING_REG 0x02

extern void fitipower_epdc_power(int enable);
#endif 
