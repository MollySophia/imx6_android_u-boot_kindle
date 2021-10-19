/*
 * inventory.c 
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
#include <post.h>
#include <i2c.h>

#ifdef CONFIG_PMIC
#include <lab126/pmic.h>
#endif

#ifdef CONFIG_PMIC_MAX77696
#include <lab126/pmic_max77696.h>
#endif
#include <asm/gpio.h>

extern const u8 *get_board_id16(void);

#ifdef CONFIG_PMIC_MAX77696
#define MAX77696_FG_REG_CONFIG        0x1D
#define MAX77696_UIC_REG_ENUCTRL      0x0C
#define MAX77696_EPD_REG_EPDINTM      0x63
#define MAX77696_RTC_REG_RTCDOWAX     0x11
#endif

#define I2C_2_ALS                     0x4A
#define I2C_2_KL05                    0x58
#define I2C_2_PROX                    0x0D        
#define I2C_2_HAPTIC                  0x59
#define HAPTIC_RECOVERY_ATTEMPTS      3
typedef struct{
    uint origin_val;
    uint test_val;
    uint now_val; 
}pmic_inventory;

/* Read register value from i2c slave.
 *
 * unsigned char saddr ----slave address
 * unsigned int reg    ----register to read
 * unsigned int *val   ----register value
 */
int pmic_i2c_read_reg(unsigned char saddr,unsigned int reg,unsigned int *val)
{
    int retry = 3;
    while(retry--)
    {
        if(board_pmic_read_reg(saddr,reg,val))
        {
            break;
        }        
    }
    if(retry <=0 )
    {
        return 1;
    }
    udelay(100);

    return 0;
}
/* Write value to i2c slave.
 *
 * unsigned char saddr ----slave address
 * unsigned int reg    ----register to write to
 * unsigned int *val   ----register value
 */
int pmic_i2c_write_reg(unsigned char saddr,unsigned int reg,unsigned int val)
{
    int retry = 3;
    while( retry-- )
    {
        if( board_pmic_write_reg(saddr,reg,val))
        {
            break;
        }        
    }
    if(retry<=0)
    {
        return 1;
    }
    udelay(100);
    return 0;
}
enum
{
    I2C_PASS           = 0,
    I2C_FAIL           = -1,
    I2C_TEST_NOT_MATCH = -2
};

#ifdef CONFIG_PMIC_MAX77696
/* test max77696 i2c slave.
 *
 * uint saddr       ----slave address
 * uchar reg        ----register to read
 * uint check_val   ----write this value to the register,then read it again then check.
 * 1. read the value from the register, save it
 * 2. write the check_val to the register, then read that register again to check the
 *    vaule is equal to the check_val or not, if equal, then pass, not will fail
 * 3. write the original value back to the register, make sure it does destroy anything
 */ 
int check_max77696 (uint saddr,uchar reg,uint check_val)
{
    int ret = I2C_FAIL;
    //int bus;   
    pmic_inventory temp;
    temp.test_val = check_val;

    //bus = i2c_get_bus_num();
    
    if( pmic_i2c_read_reg(saddr,reg,&temp.origin_val) == 0)
    {
        if( pmic_i2c_write_reg(saddr,reg,temp.test_val) == 0 )
        {
            if( pmic_i2c_read_reg(saddr,reg,&temp.now_val) == 0 )
            {
                if(temp.test_val == temp.now_val)
                {
                    ret = I2C_PASS;
                }
                else
                {
                    ret = I2C_TEST_NOT_MATCH;
                }
            }
            /** 
             Restore the original value no matter if it pass or fail
             */ 
            if( pmic_i2c_write_reg(saddr,reg,temp.origin_val) )
            {
                ret = I2C_FAIL;
            }
        }
    }
    
    switch (ret)
    {
        case I2C_PASS:
            printf("PASS\n");
            break;
        case I2C_TEST_NOT_MATCH:
            printf("Test value mismatch\n");
            break;
        case I2C_FAIL:
        default:
            printf("I2C bus failed\n");
            break;
    }
    //i2c_set_bus_num(bus);
    return ret;
}      

/* check fuel gauge.
 *
 * unsigned char saddr ----0x34
 * unsigned int reg    ----0x1d
 * unsigned int *val   ----0x5555
 */                          
int check_max77696_fuelgauge (void)
{

    printf("Max77696 FG ");
    return check_max77696(FG_SADDR,MAX77696_FG_REG_CONFIG,0x5555);
}  
/* check usb interface circuit.
 *
 * unsigned char saddr ----0x35
 * unsigned int reg    ----0x0c
 * unsigned int *val   ----0x03
 */ 
int check_max77696_uic (void)
{
    printf("\nMax77696 UIC ");
    return check_max77696(USBIF_SADDR,MAX77696_UIC_REG_ENUCTRL,0x03);
}                          
 /* check max77696 core(io/eink display supplies/..).
 *
 * unsigned char saddr ----0x3c
 * unsigned int reg    ----0x63
 * unsigned int *val   ----0x03
 */ 
int check_max77696_core (void)
{
    printf("Max77696 Core ");
    return check_max77696(PM_GPIO_SADDR,MAX77696_EPD_REG_EPDINTM,0x03);
} 

 /* check rtc.
 *
 * unsigned char saddr ----0x68
 * unsigned int reg    ----0x11
 * unsigned int *val   ----0x03
 */ 
int check_max77696_rtc (void)
{
    printf("Max77696 RTC "); 
    return check_max77696(RTC_SADDR,MAX77696_RTC_REG_RTCDOWAX,0x03);
} 
#endif

#ifdef CONFIG_LP5523_LED_CONTROL
#define LED_DRIVER_I2C_BUS_NUM	1
#define AUDIO_CODEC_I2C_BUS_NUM	2

int check_led_driver(void)
{
	int rval = 0;
	u8 tval = 0x5a;
	u8 trval = 0;
	u8 i;
	printf("LED driver ");

	rval = i2c_set_bus_num(LED_DRIVER_I2C_BUS_NUM);
	if(rval) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, LED_DRIVER_I2C_BUS_NUM);
	}
	
	rval = i2c_write(0x32, 0x3c, 1, (unsigned char *)&tval, 1);
	if(rval){
		printf("i2c_write error:%d\n",rval);
	}

	rval = i2c_read(0x32, 0x3c, 1, (unsigned char *)&trval, 1);
	if(rval){
		printf("i2c_read error:%d\n",rval);
	}

	if(tval == trval) {
		printf("PASS\n");
	}
	else {
		printf("FAILED\n");
		rval = -1;
	}

	return rval;
}
#endif

#ifdef CONFIG_AUDIO_CODEC
int check_audio_codec(void)
{
	int rval = 0;
	u8 tval = 0x43;
	u8 trval = 0;

	printf("Audio codec ");

	rval = i2c_set_bus_num(AUDIO_CODEC_I2C_BUS_NUM);
	if(rval) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, AUDIO_CODEC_I2C_BUS_NUM);
	}
	

	rval = i2c_read(0x10, 0xFF, 1, (unsigned char *)&trval, 1);
	if(rval){
		printf("i2c_read error:%d\n",rval);
	}

	if(tval == trval) {
		printf("PASS\n");
	}
	else {
		printf("FAILED\n");
		rval = -1;
	}

	return rval;
}
#endif

#ifdef CONFIG_PMIC_ROHM
#define PMIC_I2C_BUS_NUM 0
#define I2C_ADDR_PMIC_FITIPOWER 0x48
#define I2c_ADDR_PMIC_ROHM 0x4b
#define PMIC_ROHM_SWID_DS2 0x21
#define PMIC_ROHM_SWID_DS3 0x31
#define PMIC_ROHM_SWID_MP 0x41

int check_pmic_rohm(void)
{
	int rval = 0;
	u8 trval = 0;

	printf("PMIC BD71815: ");

	rval = i2c_set_bus_num(PMIC_I2C_BUS_NUM);
	if(rval) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, PMIC_I2C_BUS_NUM);
	}
	

	rval = i2c_read(I2c_ADDR_PMIC_ROHM, 0x00, 1, (unsigned char *)&trval, 1);
	if(rval){
		printf("i2c_read error:%d\n",rval);
	}

	if((trval==PMIC_ROHM_SWID_DS2) || (trval==PMIC_ROHM_SWID_DS3) || (trval==PMIC_ROHM_SWID_MP)) {
		printf("(SWID 0x%x)PASS\n",trval);
	}
	else {
		printf("(SWID 0x%x)FAILED\n",trval);
		rval = -1;
	}

	return rval;
}

int check_pmic_fitipower(void)
{
	int rval = 0;
	u8 tval = 0x01;
	u8 trval = 0;

	printf("PMIC FP9928: ");

	rval = i2c_set_bus_num(PMIC_I2C_BUS_NUM);
	if(rval) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, PMIC_I2C_BUS_NUM);
	}
	

	rval = i2c_read(I2C_ADDR_PMIC_FITIPOWER, 0x01, 1, (unsigned char *)&trval, 1);
	if(rval){
		printf("i2c_read error:%d\n",rval);
	}

	if(tval == trval) {
		printf("PASS\n");
	}
	else {
		printf("FAILED\n");
		rval = -1;
	}

	return rval;
}
#endif

#ifdef CONFIG_EANAB
#define USB_I2C_BUS_NUM 2
#define I2C_ADDR_USBSW 0x25
int check_usbsw(void)
{
	int rval = 0;
	u8 tval = 0x12;
	u8 trval = 0;

	printf("USB SW TSU6111: ");

	rval = i2c_set_bus_num(USB_I2C_BUS_NUM);
	if(rval) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, USB_I2C_BUS_NUM);
	}
	rval = i2c_read(I2C_ADDR_USBSW, 0x01, 1, (unsigned char *)&trval, 1);
	rval = i2c_read(I2C_ADDR_USBSW, 0x01, 1, (unsigned char *)&trval, 1);//FIXME, just retry
	if(rval){
		printf("i2c_read error:%d\n",rval);
	}

	if(tval == trval) {
		printf("PASS\n");
	}
	else {
		printf("FAILED\n");
		rval = -1;
	}

	return rval;
}

#ifdef CONFIG_CMD_ZFORCE
extern int bus_i2c_smb_read(void *base, uchar chip, uchar *buf, int len);
#define TOUCH_RESET_PIN (IMX_GPIO_NR(4, 5))
#define TOUCH_I2C_BASE ((void *)I2C2_BASE_ADDR)
#define TOUCH_ST_MCU 0xF0
#define TOUCH_TI_MCU 0x07
#define TOUCH_I2C_ADDR_ST 0x50
#define TOUCH_I2C_ADDR_TI 0x51
#endif
#define TOUCH_I2C_BUS_NUM 1
#define I2C_ADDR_TOUCH 0x50

int check_touch(void)
{
	int rval = 0;
	unsigned char cmd_res[2], rsp_buf[0x20];
	unsigned char i2caddr;

	printf("TOUCH: ");

	gpio_direction_output( TOUCH_RESET_PIN , 0);//reset
	udelay(200000);
	gpio_direction_output( TOUCH_RESET_PIN , 1);
	udelay(100000);

	rval = i2c_set_bus_num(TOUCH_I2C_BUS_NUM);
	if(rval) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, TOUCH_I2C_BUS_NUM);
	}
	
#ifdef CONFIG_CMD_ZFORCE
	i2caddr = TOUCH_I2C_ADDR_ST;
	rval = bus_i2c_smb_read(TOUCH_I2C_BASE,i2caddr,cmd_res, 2);
	if(rval) {
		i2caddr = TOUCH_I2C_ADDR_TI;
		rval = bus_i2c_smb_read(TOUCH_I2C_BASE,i2caddr,cmd_res, 2);
		if(rval) {
			printf("Reading failed");
			printf("FAILED\n");
			return -1;
		}
	}
	if (cmd_res[0] != 0xEE) {
		printf("Response header error");
		rsp_buf[0] = 0;
	} else {
		rval = bus_i2c_smb_read(TOUCH_I2C_BASE,i2caddr,rsp_buf,cmd_res[1]);
		if (rval) {
			printf("%s Read cmd response error\n", __FUNCTION__);
		}
	}

	if ( rsp_buf[0] == TOUCH_ST_MCU)
		printf("STM32F030K6T6 ");
	else if (rsp_buf[0] == TOUCH_TI_MCU)
		printf("TI MSP430G ");
	else
		printf("unknow ");
	
#else
	if (i2c_probe(I2C_ADDR_TOUCH) != 0)
	{
		printf("Touch device probe failed, ");
		printf("FAILED\n");
		return -1;
	}
#endif

	printf("Touch device found, ");
	printf("PASSED\n");

	return 0;
}
#endif

#define MX6_abc123_I2C1_EN	 IMX_GPIO_NR(4, 17)


int do_inventory(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	//const char *rev;
	int ret   = 0;

    /* Set i2c1 enable line gpio4_17 pull high*/
    //mxc_iomux_v3_setup_pad(MX6SL_PAD_FEC_RXD0__GPIO_4_17);
    /* set as output high */
//	gpio_direction_output(MX6_abc123_I2C1_EN, 1);

#ifdef CONFIG_PMIC_MAX77696
	ret |= check_max77696_uic();    
	ret |= check_max77696_fuelgauge();       
	ret |= check_max77696_rtc();  
	ret |= check_max77696_core();
#endif
#ifdef CONFIG_LP5523_LED_CONTROL
	ret |= check_led_driver();
#endif
#ifdef CONFIG_AUDIO_CODEC
	ret |= check_audio_codec();
#endif
#ifdef CONFIG_PMIC_ROHM
	ret |= check_pmic_rohm();
#endif

#ifdef CONFIG_PMIC_FITIPOWER
        ret |= check_pmic_fitipower();
#endif

#ifdef CONFIG_EANAB
	ret |= check_usbsw();
	ret |= check_touch();
#endif
	if(ret){
		printf("i2c device(s) inventory failed\n");
		return 0;
	}
	else{
		printf("i2c device(s) inventory pass\n");	
		return 1;
	}
}

U_BOOT_CMD(
	inventory,	CONFIG_SYS_MAXARGS,	1,	do_inventory,
	"inventory",
	""
);


