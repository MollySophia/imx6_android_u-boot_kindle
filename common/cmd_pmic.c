/*
 * cmd_pmic.c  
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

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <lab126/pmic.h>
#include <lab126/pmic_max77696.h>

/*
 * PMIC read/write
 *
 * Syntax:
 *   pmic wr {saddr} {register} {value}
 *     {saddr} - pmic slave address in  hex
 *   	{register} - pmic register id in hex
 *   	{value} - hex or 0b binary value
 *   pmic rd {saddr} {register}
 *    {saddr} - pmic slave address in  hex
 *   	{register} - pmic register id in decimal or hex
 *   	{value} - hex or 0b binary value
 *   pmic wor {saddr} {register} {bit_high} {bit_low} {value}
 *   	{register} - pmic register id in decimal or hex
 *   	{value} - hex or 0b binary value
 */

int do_pmic (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int res = 1;  /* default is failure */
    unsigned int val;
    unsigned int reg, saddr;
    

    /* we're not repeating, parse commands */
    if (strcmp(argv[1], "init") == 0) {

	res = pmic_init();
	if (res) {
	    res = 0;
	} else {
	   printf("Error on init!\n");
	   res = 1;   
	}
	
    } else if(strcmp(argv[1], "wr") == 0 && argc == 5) {
	
	saddr = simple_strtoul(argv[2], NULL, 16);
	reg = simple_strtoul(argv[3], NULL, 16);
	val = simple_strtoul(argv[4], NULL, 16);
			
	res = pmic_write_reg(saddr, reg, val);	
	if (res) {
		printf("write saddr=0x%x reg=0x%x val=0x%x\n", saddr, reg, val);
	    res = 0;
	} else {
	   printf("Error on write!\n");
	   res = 1;  
	}
       
    } else if(strcmp(argv[1], "rd") == 0 && argc == 4) {
	
	saddr = simple_strtoul(argv[2], NULL, 16);
	reg = simple_strtoul(argv[3], NULL, 16);

	res = pmic_read_reg(saddr, reg, &val);
	if (res) {
	    printf("read saddr=0x%x reg=0x%x val=0x%x\n",saddr, reg, val);
	    res = 0;
	} else {
	    printf("Error on read!\n");
	    res = 1;
	}
    }
    else if(strcmp(argv[1], "wor") == 0 && argc == 7) {
	unsigned bit_h, bit_l, reg, value, mask, i;

	saddr=simple_strtoul(argv[2], NULL, 16);
	reg=simple_strtoul(argv[3], NULL, 16);
	
	bit_h=simple_strtoul(argv[4], NULL, 0);
	bit_l=simple_strtoul(argv[5], NULL, 0);
	value=simple_strtoul(argv[6], NULL, 16);

	if (bit_h > 32 || bit_l > 32 || bit_h < bit_l) {
	    printf("Invalid parameters\n");
	    return 1;
	}
	
	printf("reg=%d h=%d l=%d val=%d\n", reg, bit_h, bit_l, value);

	mask = 0;
	for (i = bit_l; i <= bit_h; i++) {
	    mask |= (1 << i);
	}
	
	res = pmic_wor_reg(saddr, reg, value, mask);
	if (res) {
	    res = 0;
	} else {
	   printf("Error on write!\n");
	   res = 1;  
	}
    } 
    else {
		cmd_usage(cmdtp);
	return 1;
    }

    return res;
}

/***************************************************/

U_BOOT_CMD(
	pmic,	7,	1,	do_pmic,
	"pmic    - PMIC utility commands",
	"[wr|rd|wor] args...\n"
	"wr <saddr> <reg> <hexvalue>\n"
	"    - write a register\n"
	"rd <saddr> <reg>\n"
	"    - read a register\n"
	"wor <saddr> <reg> <bit_high> <bit_low> <hexvalue>\n"
	"    - set some bits in a register (write-OR)\n"
);


