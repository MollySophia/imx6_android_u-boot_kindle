/*
 * cmd_vni.c  
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

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <lab126/pmic.h>
#include <lab126/pmic_max77696.h>
#include <asm/io.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <boardid.h>
#include <mmc.h>

#ifdef CONFIG_PMIC_ROHM
#include <lab126/pmic_rohm.h>
#endif

#define atoi(x)         simple_strtoul(x,NULL,10)

extern int pmic_fg_otp_check(void);

int do_vni (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int res = 0;  /* default is pass */
	unsigned short val = 0;
	int curr, temp;
	if(argc < 2)
	{
		cmd_usage(cmdtp);
		return 1;
	}	
	
		
#ifdef CONFIG_PMIC_MAX77696
	int brightness = 0;
	if(strcmp(argv[1], "vcom") == 0) {
	    if(strcmp(argv[2], "set") == 0 ){
			if(pmic_vcom_set(argv[3]) != PMIC_VNI_TEST_PASS ) {
				printf("Enable Vcom	failed!\n");
				res = 1;
			}else {									
				printf("Turn on VCOM\n");
			}
	    }
	    else if(strcmp(argv[2], "off") == 0 ) {
			if( pmic_enable_vcom(0) != PMIC_VNI_TEST_PASS )	{
				printf("disable Vcom  failed!\n");
				res = 1;
			}else{
				printf("Turn off VCOM\n");
			}
	   }else {
			cmd_usage(cmdtp);
			return 1;
	  }
	}else if(strcmp(argv[1], "batt") == 0) {
	/*will add read battery info here*/	
	    if(strcmp(argv[2], "vol") == 0 ){
			if(!pmic_fg_read_voltage(&val)) {
				printf("Read voltage failed!\n");
				res = 1;
			}else {									
				printf("FG voltage =%dmV\n",val);
			}
	    }
	    else if(strcmp(argv[2], "curr") == 0 ) {
			if(!pmic_fg_read_avg_current(&curr))	{
				printf("Read FG Current failed!\n");
				res = 1;
			}else{
				printf("FG Current = %dmA\n",curr);
			}
	   }
	   else if(strcmp(argv[2], "temp") == 0 ){
		    if(!pmic_fg_read_temperature(&temp)) {
				 printf("Read FG temperature failed!\n");
				 res = 1;
			 }else {								 
				printf("FG Temperature = %dC\n",temp);
			 }
		 }
	   else if(strcmp(argv[2], "cap") == 0 ) {
			 if(!pmic_fg_read_capacity(&val)) {
				 printf("Read FG capacity  failed!\n");
				 res = 1;
			 }else{
				 printf("FG Capacity = %d Percent\n",val);
			 }
		}
	   else {
		   cmd_usage(cmdtp);
		   return 1;

	   	}
	   
	}else if(strcmp(argv[1], "chg") == 0) {
  	       if(strcmp(argv[2], "trickle") == 0 ){
                    printf("Set charge current to 66.7mA\n");
                    pmic_charger_set_current(MAX77696_CHARGER_CURRENT_TRICKLE);
  	       }else if(strcmp(argv[2], "default") == 0 ) {
  	            printf("Set charge current to 300mA\n");
                    pmic_charger_set_current(MAX77696_CHARGER_CURRENT_DEFAULT);
               }else if(strcmp(argv[2], "fast") == 0 ) {
                    printf("Set charge current to 466mA\n");
	            pmic_charger_set_current(MAX77696_CHARGER_CURRENT_FAST);
               }else {
	            cmd_usage(cmdtp);
	            return 1;
	            }
	}else if(strcmp(argv[1], "gpio") == 0) {
		if ((argc == 4) && (strcmp(argv[2], "3gm_shutdown") == 0)) {
			curr = atoi(argv[3]) ? 1 : 0;
			printf("set 3gm_shutdown to %d\n", curr);
			gpio_direction_output(IMX_GPIO_NR(4,0), curr);
		}

	} else if(strcmp(argv[1], "display") == 0) {
		unsigned int rail_enable;

		if (strcmp(argv[2], "on") == 0) {
			rail_enable = 1;
		} else {
			rail_enable = 0;
		}
		printf("set LSW4 to %d\n", rail_enable);
		pmic_enable_display_power_rail(rail_enable);

	}else if(strcmp(argv[1], "fl") == 0) {	         
             brightness = simple_strtoul(argv[2], NULL, 10); 
			 printf("set front light brightness:%d\n",brightness);
             pmic_fl_set(brightness);

    } else if(strcmp(argv[1], "fg") == 0) {
       if (strcmp(argv[2], "otpchk") == 0) {
            printf("MAX77696-FG OTP TIMER TEST\n");
            pmic_fg_otp_check();
       } else {
           cmd_usage(cmdtp);
           return 1;
       }
	}else {
	       cmd_usage(cmdtp);
	       return 1;
	}
#endif
#ifdef CONFIG_PMIC_ROHM
       if(strcmp(argv[1], "vcom") == 0) {
            if(strcmp(argv[2], "set") == 0 ){
                        if(pmic_vcom_set(argv[3]) != PMIC_VNI_TEST_PASS ) {
                                printf("Enable Vcom     failed!\n");
                                res = 1;
                        }else {
                                printf("Turn on VCOM\n");
                        }
            }
            else if(strcmp(argv[2], "off") == 0 ) {
                        if( pmic_enable_vcom(0) != PMIC_VNI_TEST_PASS ) {
                                printf("disable Vcom  failed!\n");
                                res = 1;
                        }else{
                                printf("Turn off VCOM\n");
                        }
           }else {
                        cmd_usage(cmdtp);
                        return 1;
          }
        }else if(strcmp(argv[1], "batt") == 0) {
       /*will add read battery info here*/
            if(strcmp(argv[2], "vol") == 0 ){
                        if(!pmic_fg_read_voltage(&val)) {
                                printf("Read voltage failed!\n");
                                res = 1;
                        }else {
                                printf("FG voltage =%dmV\n",val);
                        }
            }
            else if(strcmp(argv[2], "curr") == 0 ) {
                        if(!pmic_fg_read_avg_current(&curr))    {
                                printf("Read FG Current failed!\n");
                                res = 1;
                        }else{
                                printf("FG Current = %dmA\n",curr);
                        }
           }
           else if(strcmp(argv[2], "temp") == 0 ){
                    if(!pmic_fg_read_temperature(&temp)) {
                                 printf("Read FG temperature failed!\n");
                                 res = 1;
                         }else {
                                printf("FG Temperature = %dC\n",temp);
                         }
                 }
           else if(strcmp(argv[2], "cap") == 0 ) {
                         if(!pmic_fg_read_capacity(&val)) {
                                 printf("Read FG capacity  failed!\n");
                                 res = 1;
                         }else{
                                 printf("FG Capacity = %d Percent\n",val);
                         }
                }
           else {
                   cmd_usage(cmdtp);
                   return 1;
                }

        }else if(strcmp(argv[1], "chg") == 0) {
               if(strcmp(argv[2], "trickle") == 0 ){
                    printf("Set charge current to 100mA\n");
                    pmic_charger_set_current(ROHM_CHARGER_CURRENT_TRICKLE);
               }else if(strcmp(argv[2], "default") == 0 ) {
                    printf("Set charge current to 300mA\n");
                    pmic_charger_set_current(ROHM_CHARGER_CURRENT_DEFAULT);
               }else if(strcmp(argv[2], "fast") == 0 ) {
                    printf("Set charge current to 500mA\n");
                    pmic_charger_set_current(ROHM_CHARGER_CURRENT_FAST);
               }else {
                    cmd_usage(cmdtp);
                    return 1;
                    }
        }else if(strcmp(argv[1], "display") == 0) {
                unsigned int rail_enable;

				if (strcmp(argv[2], "temp") == 0) {
					if(!pmic_read_epdc_temperature(&temp)) {
						printf("Read Display temperature failed!\n");
							res = 1;
					}else {
						printf("Display Temperature = %dC\n",temp);
					}
				}
				else {
					if (strcmp(argv[2], "on") == 0) {
						rail_enable = 1;
					} else {
						rail_enable = 0;
					}
                	printf("set EINK power to %d\n", rail_enable);
                	pmic_enable_display_power_rail(rail_enable);
				}

        }else {
               cmd_usage(cmdtp);
               return 1;
        }
#endif
	return res;
}

#ifdef CONFIG_WAN_CMD

void mx60_wan4v2_ldo_gpio(int on)
{
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRCTRL3__GPIO_2_10 | MUX_PAD_CTRL(NO_PAD_CTRL));
	/* set as output high */
	gpio_direction_output(IMX_GPIO_NR(2,10), on);
}
void mx60_wan4v2_fet_gpio(int on)
{
	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRCTRL1__GPIO_2_8 | MUX_PAD_CTRL(NO_PAD_CTRL));
	/* set as output high */
	gpio_direction_output(IMX_GPIO_NR(2,8), on);
}

int do_wan4v2 (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	
	if(strcmp(argv[1], "ldo") == 0) {
	    mx60_wan4v2_fet_gpio(0);//turn off fet first
	    udelay(500000);//delay 500ms to wait for fet is off
	    mx60_wan4v2_ldo_gpio(1);//turn on ldo
	    printf("Turn on 3GM_LDO_EN\n");
	}
	else if (strcmp(argv[1], "fet") == 0) {
	    mx60_wan4v2_ldo_gpio(0);//turn off ldo
	    udelay(500000);//delay 500ms to wait for fet is off
	    mx60_wan4v2_fet_gpio(1);//turn on fet 
	    printf("Turn on 3GM_FET_EN\n");
	}
	else if (strcmp(argv[1], "off") == 0) {
	    mx60_wan4v2_ldo_gpio(0);//turn off ldo
	    udelay(500000);//delay 500ms to wait for fet is off
	    mx60_wan4v2_fet_gpio(0);//turn off fet 
	    printf("Turn off 3GM_FET_EN and 3GM_LDO_EN\n");
	}
	else {
	    cmd_usage(cmdtp);
	    return 1;

	}
        return 0;
}

U_BOOT_CMD(
        wan4v2, 2,      0,      do_wan4v2,
        "wan4v2    - WAN module 4V2 control",
        "[fet|ldo|off] args...\n"
        "wan4v2 fet\n"
        "    - turn off ldo then turn on fet\n"
        "wan4v2 ldo\n"
        "    - turn off fet then turn on ldo \n"
        "wan4v2 off\n"
        "    - turn off fet and ldo\n"
);

#endif
/***************************************************/

U_BOOT_CMD(
	vni,	5,	0,	do_vni,
	"vni    - vni pmic tests",
	"[vcom|batt|chg|fg] args...\n"
	"vcom set <vaule> or <off>\n"
	"    - set vcom value/turn off vcom\n"
	"batt [vol|temp|curr|cap]\n"
	"    - read battery infomation\n"
	"chg [trickle|default|fast]\n"
	"    - set fast charge current\n"
	"gpio 3gm_shutdown [0/1]\n"
	"display [on/off/temp]\n"
	"fg otpchk\n"
);


////////////////////////////////////////////////////////////////////
int do_read_mmc_revision(void)
{
	struct mmc *mmc;
	int rev;
	
	mmc = find_mmc_device(1);
	if (mmc) 
        {
	    mmc_init(mmc);
	    rev = (mmc->cid[2] >> 16) & 0xff;	
	    printf("eMMC Revision:0x%x\n",rev);	
	    return 0;
	} else {
	    printf("no mmc device at slot 1\n");
	    return 1;
	}
	return 1;
}



typedef struct _emmc_vendors_list
{
	int vendor_id;
	char *vendor_name;
}EMMC_VENDORS_LIST;

EMMC_VENDORS_LIST emmc_vendors[] =
{
	{0x11, "Toshiba"},
	{0x15, "Samsung"},
	{0x45, "Sandisk"},
	{0x90, "Hynix"},
	{0xfe, "Micron"},
	{0, NULL}
};




int do_read_mmc_mfgid(void)
{
	struct mmc *mmc;
	int i = 0;
	mmc = find_mmc_device(1);

	if (mmc) 
        {
	    mmc_init(mmc);
	    int mfgid = mmc->cid[0] >> 24;
	    while(emmc_vendors[i++].vendor_id != 0)
	    {
		if(emmc_vendors[i].vendor_id == mfgid)
		{
			printf("eMMC vendor: %s\n",emmc_vendors[i].vendor_name);
			return 0;
		}
	    }	
	    printf("No eMMC vendor is found,mfgid=%x\n",mfgid);	
	    return 1;
	} else {
	    printf("no mmc device at slot 1\n");
	    return 1;
	}
	return 1;
}

//*********************************************************************/

#define _2K_MB_EMMC_SIZE (2*1024)
#define _4K_MB_EMMC_SIZE (4*1024)
#define _1MB (1024*1024)

int do_read_mmc_capacity(void)
{
	struct mmc *mmc;

	mmc = find_mmc_device(1);

	if (mmc) 
        {
	    mmc_init(mmc);
		int mmccap = mmc->capacity/_1MB;

	    if ( mmccap > 0 && mmccap <= _2K_MB_EMMC_SIZE )
	    {
		    printf("eMMC Capacity:2GB\n");
	    }
	    else if ( mmccap > _2K_MB_EMMC_SIZE  && mmccap <= _4K_MB_EMMC_SIZE )
	    {
		    printf("eMMC Capacity:4GB\n");
	    }else
	    {
		    printf("Capacity: %lld\n", mmc->capacity);
	    }
	    return 0;
	} else {
	    printf("no mmc device at slot 1\n");
	    return 1;
	}
	return 1;
}


int do_vni_mmc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	if(strcmp(argv[1], "cap") == 0) {
		return do_read_mmc_capacity();

	}else if (strcmp(argv[1], "mfgid") == 0) {
		return do_read_mmc_mfgid();
        }else if (strcmp(argv[1], "ver") == 0) {
		return do_read_mmc_revision();
	}else {
		cmd_usage(cmdtp);
		return 1;
	}
	return 1;
}


U_BOOT_CMD(
	vnimmc, 2, 0, do_vni_mmc,
	"display MMC mfgid",
	"[cap|mfgid|ver] args...\n"
	"cap\n"
	"    - get the eMMC capacity\n"
	"mfgid\n"
	"    - get the eMMC vendor name \n"
	"ver\n"
	"    - get the eMMC FW version \n"
);

unsigned int *mr5_bist = (unsigned int *)0x80800000; //value is got from prod which run@IRAM
unsigned int *mr6_bist = (unsigned int *)0x80800004;
int do_vni_ddr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	
	switch(*mr5_bist){
	   case 0x01:
		printf("DDR vendor: Samsung(MR5=0x%x),Rev=0x%x\n",*mr5_bist,*mr6_bist);
		break;

	   case 0x03:
		printf("DDR vendor: Micron(MR5=0x%x),Rev=0x%x\n",*mr5_bist,*mr6_bist);
		break;

	   case 0x06:
		printf("DDR vendor: Hynix(MR5=0x%x),Rev=0x%x\n",*mr5_bist,*mr6_bist);
		break;

	   default:
		printf("DDR vendor: unknow(mr5=0x%x),Rev=0x%x\n",*mr5_bist,*mr6_bist);
		break;
	}
        return 1;
}


U_BOOT_CMD(
        vniddr, 1, 1, do_vni_ddr,
        "display ddr MR5/MR6",
	" "
);

#ifdef CONFIG_WIFI_CMD
int do_wifi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
//	struct mmc *mmc;
//	imx_iomux_v3_setup_pad(MX6_PAD_EPDC_PWRCTRL3__GPIO_2_10 | MUX_PAD_CTRL(NO_PAD_CTRL));
     if(strcmp(argv[1], "off") == 0 ){
       printf("Turn off WiFi moudle\n");
       gpio_direction_output(IMX_GPIO_NR(2,16), 0);
       gpio_direction_output(IMX_GPIO_NR(2,15), 0);
       gpio_direction_output(IMX_GPIO_NR(2,19), 0);
     }else
     if(strcmp(argv[1], "on") == 0 ){
       printf("Turn on WiFi moudle\n");
//	gpio_direction_output(IMX_GPIO_NR(2,18), 1);
        gpio_direction_output(IMX_GPIO_NR(2,15), 1);
	udelay(250000);
        gpio_direction_output(IMX_GPIO_NR(2,19), 1);
      }else{
         cmd_usage(cmdtp);
         return 0;
      }
	udelay(500000);

#if 0	
	mmc = find_mmc_device(2);
	if (mmc)
        {
            mmc_init(mmc);
	//to do something here?
            return 0;
        } else {
            printf("no mmc device at slot 1\n");
            return 1;
        }
#endif
      return 1;
}

U_BOOT_CMD(
        wifi, 2,      1,      do_wifi,
        "wifi    - WiFi module power control",
        "[on | off]\n"
);

#define HALL_SENSOR_IS_NOT_DETECT      0x1
#define HALL_SENSOR_IS_DETECT	       0x0
#define MX6_EANAB_HALL_SNS IMX_GPIO_NR(4, 16)
static int do_hall (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
       int rval,hall_val;
       rval = gpio_direction_input(MX6_EANAB_HALL_SNS);
       if ( rval != 0 ) {
            printf("%s gpio_direction_input(MX6_EANAB_HALL_SNS) failed, rval %d\n", __func__, rval);
            return rval;
       }
       hall_val = gpio_get_value(MX6_EANAB_HALL_SNS);
       if( strcmp(argv[1], "not_detect") == 0 ) {
            if( hall_val == HALL_SENSOR_IS_NOT_DETECT ) {// test hall not detect status 
                 printf("HALL NOT_DETECT TEST PASS\n");

            }else {
                 printf("HALL NOT_DETECT TEST FAIL\n");
           }


        }else if ( strcmp(argv[1], "detect") == 0 ) {
           if( hall_val == HALL_SENSOR_IS_DETECT ) { // test hall detect status 
                 printf("HALL DETECT TEST PASS\n");

           }else {
                 printf("HALL DETECT TEST FAIL\n");
           }

        }
        else {
            cmd_usage(cmdtp);
            return 1;

        }
        return 0;
}


U_BOOT_CMD(
        hall, 2,      0,      do_hall,
        "hall     - hall status test",
        "[not_detect | detect]\n"
);


#endif



