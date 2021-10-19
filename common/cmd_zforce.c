#include <common.h>
#include <command.h>
#include <asm/arch/imx-regs.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/gpio.h>

struct touchId {
	unsigned char id;
	unsigned char i2caddr;
	char name[20];
};

struct touchId supported_touch[] = {
	{0xF0, 0x50, "ST MCU touch"},
	{0x07, 0x51, "TI MCU touch"},
	{0x07, 0x50, "TI MCU touch"}
};
#define TOUCH_NUMBER (sizeof(supported_touch) / sizeof(struct touchId))
unsigned int installed_touch = TOUCH_NUMBER;

#define ZFORCE_I2C_BUS_NUM 1
#define ZFORCE_I2C_ADDR 0x50
#define ZFORCE_I2C_BASE ((void *)I2C2_BASE_ADDR)
#define ZFORCE_ERROR -1
#define ZFORCE_PASS 0

#define CMD_LEAD_BYTE 0xEE
#define RES_HEAD_LENGTH 0x02
#define RES_HEAD_LEAD_OFFSET 0x00
#define RES_HEAD_LENGTH_OFFSET 0x01
#define LEAD_BYTE_INDEX 0x00
#define RES_LEAD_BYTE 0xEE
#define CMD_RES_DELAY 500000

#define ST_LEAD_BYTE 0xEF
#define ST_BOOT_COMPLETE 0xF0
#define ST_OPENSHORT_CHECK_LENGTH 12

#define MCU_INDEX 0
#define ST_MCU 0xF0
#define TI_MCU 0x07

#define ZFORCE_RESET_PIN (IMX_GPIO_NR(4, 5))

extern int bus_i2c_smb_read(void *base, uchar chip, uchar *buf, int len);
extern int bus_i2c_smb_write(void *base, uchar chip, const uchar *buf, int len);

	char * open_short_Xaxis_led_pd_lookup_table[46] = {  \
	"OPEN_PD328","OPEN_PD327","OPEN_PD326","OPEN_PD325","OPEN_PD324","OPEN_PD323","OPEN_PD322","OPEN_PD321",         \
        "OPEN_PD320","OPEN_PD319","OPEN_PD318","OPEN_PD317","SHORT_PD328","SHORT_PD327","SHORT_PD326","SHORT_PD325",      \
        "SHORT_PD324","SHORT_PD323","SHORT_PD322","SHORT_PD321","SHORT_PD320","SHORT_PD319","SHORT_PD318","SHORT_PD317", \
	"SHORT_IR326","SHORT_IR325","SHORT_IR324","SHORT_IR323","SHORT_IR322","SHORT_IR321","SHORT_IR320","SHORT_IR319", \
        "SHORT_IR318","SHORT_IR317","SHORT_IR316","OPEN_IR326","OPEN_IR325","OPEN_IR324","OPEN_IR323","OPEN_IR322",      \
        "OPEN_IR321","OPEN_IR320","OPEN_IR319","OPEN_IR318","OPEN_IR317","OPEN_IR316",                                   \	
	};
	char * open_short_Yaxis_led_pd_lookup_table[62] = { \
	"OPEN_PD316","OPEN_PD315","OPEN_PD314","OPEN_PD313","OPEN_PD312","OPEN_PD311","OPEN_PD310","OPEN_PD309",         \
        "OPEN_PD308","OPEN_PD307","OPEN_PD306","OPEN_PD305","OPEN_PD304","OPEN_PD303","OPEN_PD302","OPEN_PD301",         \
	"SHORT_PD316","SHORT_PD315","SHORT_PD314","SHORT_PD313","SHORT_PD312","SHORT_PD311","SHORT_PD310","SHORT_PD309", \
        "SHORT_PD308","SHORT_PD307","SHORT_PD306","SHORT_PD305","SHORT_PD304","SHORT_PD303","SHORT_PD302","SHORT_PD301", \	
	"SHORT_IR315","SHORT_IR314","SHORT_IR313","SHORT_IR312","SHORT_IR311","SHORT_IR310","SHORT_IR309","SHORT_IR308", \
        "SHORT_IR307","SHORT_IR306","SHORT_IR305","SHORT_IR304","SHORT_IR303","SHORT_IR302","SHORT_IR301","OPEN_IR315",  \
        "OPEN_IR314","OPEN_IR313","OPEN_IR312","OPEN_IR311","OPEN_IR310","OPEN_IR309","OPEN_IR308","OPEN_IR307",         \
        "OPEN_IR306","OPEN_IR305","OPEN_IR304","OPEN_IR303","OPEN_IR302","OPEN_IR301",                                   \
	};

static void zforce_print_msg(unsigned char *msg_type, unsigned char *msg, int len)
{
	int i;

	printf("%s: ", msg_type);
	for(i=0; i< len; i++) {
		if((i % 16) == 0)
			printf("\n");
			printf("0x%02X,",msg[i]);
	}
	printf("\n");
}

void reset_touch(void)
{
	unsigned char value;
	int ret;	
	
	i2c_set_bus_num(0);
	if (i2c_read(0x4b, 0x12, 1, &value, 1)) {
		printf("Read pmic error!\n");
	}
	
	value = value & 0xF0;	

	ret = i2c_write(0x4b, 0x12, 1, (unsigned char *)&value, 1);//LDO5
	udelay(200000);
	value = value | 0x04;
	ret = i2c_write(0x4b, 0x12, 1, (unsigned char *)&value, 1);//LDO5
	if( ret )
		printf("Write pmic error\n");

	udelay(100000);
	
	gpio_direction_output( ZFORCE_RESET_PIN , 0);//reset
	udelay(200000);
	gpio_direction_output( ZFORCE_RESET_PIN , 1);
	udelay(100000);
}

int zforce_cmd(unsigned char * cmd, int cmd_len, unsigned char **res, int *res_len)
{
	int ret = ZFORCE_PASS;
	unsigned char *temp;
	unsigned char cmd_res[RES_HEAD_LENGTH];
	int res_length;

	*res = NULL;
	*res_len = 0;

	ret = i2c_set_bus_num(ZFORCE_I2C_BUS_NUM);
	if(ret) {
		printf("%s Cannot set i2c bus: %d\n", __FUNCTION__, ZFORCE_I2C_BUS_NUM);
		return ZFORCE_ERROR;
	}
	
	if(NULL != cmd && cmd_len != 0)
	{
		ret = bus_i2c_smb_write(ZFORCE_I2C_BASE,supported_touch[installed_touch].i2caddr, cmd,cmd_len);
		if(ret) {
			printf("%s Sending cmd error\n", __FUNCTION__);
			return ZFORCE_ERROR;
		}
		udelay(CMD_RES_DELAY);
	}

	ret = bus_i2c_smb_read(ZFORCE_I2C_BASE,supported_touch[installed_touch].i2caddr,cmd_res,RES_HEAD_LENGTH);
	if(ret) {
		printf("%s Read cmd response header error\n", __FUNCTION__);
		return ZFORCE_ERROR;
	}
	
//	printf("0x%02X,0x%02X\n",cmd_res[RES_HEAD_LEAD_OFFSET],cmd_res[RES_HEAD_LENGTH_OFFSET]);
        
	if( cmd_res[RES_HEAD_LEAD_OFFSET] != (unsigned char)RES_LEAD_BYTE )
	{
		printf("%s Response leading byte error\n", __FUNCTION__);
		return ZFORCE_ERROR;
	}

	temp = malloc(cmd_res[RES_HEAD_LENGTH_OFFSET]);
	if(NULL == temp)
	{
		printf("%s No memmory for response data\n", __FUNCTION__);
		return ZFORCE_ERROR;
	}

	res_length = (int)cmd_res[RES_HEAD_LENGTH_OFFSET];
	ret = bus_i2c_smb_read(ZFORCE_I2C_BASE,supported_touch[installed_touch].i2caddr,temp,res_length);
	if(ret) {
		printf("%s Read cmd response error\n", __FUNCTION__);
		*res = temp;
		*res_len = res_length;
		return ZFORCE_ERROR;
	}
        
	*res = temp;
	*res_len = res_length;
	return ret;
}

static int identify_touch(void)
{

	unsigned char *res_buff;
	int res_length;
	int ret = 0;
	int timeout=10;
	unsigned char tmpi2caddr;
	int i;

	reset_touch();
	
	i2c_set_bus_num(ZFORCE_I2C_BUS_NUM);
	for(i = 0; i < TOUCH_NUMBER; i++) {
		if (i2c_probe(supported_touch[i].i2caddr) == 0) {
			installed_touch = i;
			break;
		}
	}
	if (i == TOUCH_NUMBER) {
		printf("No supported MCU touch found\n");
		return ZFORCE_ERROR;
	}
	
	udelay(100000);
	gpio_direction_output( ZFORCE_RESET_PIN , 0);
	udelay(200000);
	gpio_direction_output( ZFORCE_RESET_PIN , 1);
	udelay(100000);

	do {
		ret = zforce_cmd(NULL,0,&res_buff,&res_length);
		if(ret) {
			printf("%s Read Boot Complete Notification error, retry...\n", __FUNCTION__);
			if(res_buff != NULL)
				free(res_buff);
		}
	} while ((--timeout > 0) && (ret != ZFORCE_PASS));
	
	//zforce_print_msg((unsigned char *)"Boot Complete Notification", res_buff, res_length);
	
	tmpi2caddr = supported_touch[installed_touch].i2caddr;
	for (i = 0; i < TOUCH_NUMBER; i++) {
		if ( (res_buff[MCU_INDEX] == supported_touch[i].id) && (tmpi2caddr == supported_touch[installed_touch].i2caddr)) {
			installed_touch = i;
			break;
		}
	}
	if (i == TOUCH_NUMBER) {
		installed_touch = TOUCH_NUMBER;
		printf("Installed MCU touch has unsupported ID (%x)\n", res_buff[MCU_INDEX]);
		ret = ZFORCE_ERROR;
	}

	if(res_buff != NULL)
		free(res_buff);

	return ret;
}
#define DATA_OFFSET 3	

int get_1bit(char buf, int n)  
{  
        return (buf >> n) & 0x01;  
}

	
static int do_ti_zforce(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char cmd_init[3]={0xEE,0x01,0x01};
	unsigned char cmd_openshort_x[4]={0xEE,0x02,0x21,0x00};
	unsigned char cmd_openshort_y[4]={0xEE,0x02,0x21,0x01};
	int res_length;
	unsigned char *res_buff,osdata;
	int ret_x=0, ret_y = 0;
	int i, j;

	ret_x = zforce_cmd(cmd_init,3,&res_buff,&res_length);
	if(ret_x) {
		printf("%s Init cmd failed\n",__FUNCTION__);
		goto EXIT_ERROR;
	}
	
	if (res_buff != NULL)
		free(res_buff);

	ret_x = zforce_cmd(cmd_openshort_x,4,&res_buff,&res_length);
	if(ret_x) {
		printf("%s X_Axis open/short  cmd error\n", __FUNCTION__);
		goto EXIT_ERROR;
	}
	
	zforce_print_msg((unsigned char *)"Get X_Axis data respeonse:",res_buff,res_length);

	if((res_buff[0] != 0x21) || (res_buff[1] != 0)) {
		printf("X_Axis open/short wrong response.\n");
		ret_x = ZFORCE_ERROR;
	} else {
		for(i = 0; i < (res_length - DATA_OFFSET); i++) {
			if(( osdata = res_buff[ DATA_OFFSET + i ] ) != 0) {

				for(j=0;j<8;j++)
				{			
					if( get_1bit(osdata,j) != 0 )
						      printf("X_Axis open/short failed at:%s\n", open_short_Xaxis_led_pd_lookup_table[i*8 + j]);
				}
				
				ret_x = ZFORCE_ERROR;			
			}
		}
	}
	if(ret_x) {
		printf("X_Axis open/short Test: FAIL.\n");
		//goto EXIT_ERROR; should continue to test the Y_Axis
	}
	else
		printf("X_Axis open/short Test: PASS.\n");
	
	if (res_buff != NULL)
		free(res_buff);
	
	ret_y = zforce_cmd(cmd_openshort_y,4,&res_buff,&res_length);
	if(ret_y) {
		printf("%s Y_Axis open/short  cmd error\n", __FUNCTION__);
		goto EXIT_ERROR;
	}
	
	zforce_print_msg((unsigned char *)"Get Y_Axis data respeonse:",res_buff, res_length);
	
	if((res_buff[0] != 0x21) || (res_buff[1] != 1)) {
		printf("Y_Axis open/short wrong response.\n");
		ret_y = ZFORCE_ERROR;
	} else {
		for(i = 0; i < (res_length - DATA_OFFSET); i++) 
                {
			if((osdata = res_buff[ DATA_OFFSET + i ]) != 0) 
                        {
				for(j=0;j<8;j++)
				{			
					if( get_1bit(osdata,j) != 0 )
						printf("Y_Axis open/short failed at:%s\n", open_short_Yaxis_led_pd_lookup_table[i*8 + j]);
				}
				ret_y = ZFORCE_ERROR;
			}		
		}
	}
	if(ret_y) {
		printf("Y_Axis open/short Test: FAIL.\n");
		goto EXIT_ERROR;
	}
	else
		printf("Y_Axis open/short Test: PASS.\n");

	if (res_buff != NULL)
		free(res_buff);
	if(ret_x || ret_y)
		
		return ZFORCE_ERROR;
	else
		return ZFORCE_PASS;	
EXIT_ERROR:
	if (res_buff != NULL)
		free(res_buff);
	return ZFORCE_ERROR;
}

U_BOOT_CMD(
	ti_zforce,	1,	1,	do_ti_zforce,
	"ti touch zforce opensort test",
	""
);


static int do_st_zforce(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char cmd_disablezforce[12] = {0xEE,0x0A,0xEE,0x08,0x40,0x02,0x01,0x00,0x65,0x02,0x80,0x00};
	unsigned char cmd_openshort[10]={0xEE,0x08,0xEE,0x06,0x40,0x02,0x00,0x00,0x6A,0x00};
	unsigned char *res_buff = NULL, osdata;
	int res_length;
	int ret = 0;
	int i,j;

	printf("DisableZforce:\n");
	zforce_print_msg((unsigned char *)"Request", cmd_openshort, 12);
	ret = zforce_cmd(cmd_disablezforce,12,&res_buff,&res_length);
	if(ret) {
        printf("%s DisableZforceCore cmd error\n", __FUNCTION__);
		if(res_buff != NULL)
			free(res_buff);
        return ZFORCE_ERROR;
	}
	zforce_print_msg((unsigned char *)"Response",res_buff,res_length);

	if(res_buff != NULL)
		free(res_buff);

	printf("Open/Short:\n");
	zforce_print_msg((unsigned char *)"Request", cmd_openshort,10);
	ret = zforce_cmd(cmd_openshort,10,&res_buff,&res_length);
	if(ret) {
		printf("%s Openshort cmd error\n", __FUNCTION__);
		if(res_buff != NULL)
			free(res_buff);
		return ZFORCE_ERROR;
	}
	zforce_print_msg((unsigned char *)"Response",res_buff,res_length);

#define OPEN_SHORT_RSP_LENGTH 0x25
#define LED_OPEN_TAG_INDEX 15
#define LED_SHORT_TAG_INDEX 22
#define PD_SHORT_TAG_INDEX 29

	if ( (res_length != (res_buff[1]+2)) || (res_buff[6] != 0x6A)) {
		printf("Wrong open/short response.%x,%x\n", res_length,res_buff[1]);
		if(res_buff != NULL)
			free(res_buff);
		return ZFORCE_ERROR;
	}

	zforce_print_msg((unsigned char *)"LED Open data", &(res_buff[LED_OPEN_TAG_INDEX+3]), res_buff[LED_OPEN_TAG_INDEX+1] - 1);
	zforce_print_msg((unsigned char *)"LED Short data", &(res_buff[LED_SHORT_TAG_INDEX+3]), res_buff[LED_SHORT_TAG_INDEX+1] - 1);
	zforce_print_msg((unsigned char *)"PD Short data", &(res_buff[PD_SHORT_TAG_INDEX+3]), res_buff[PD_SHORT_TAG_INDEX+1] - 1);

	res_length = res_buff[LED_OPEN_TAG_INDEX+1] - 1;
	for (i = 0; i < res_length; i++) {
		osdata = res_buff[LED_OPEN_TAG_INDEX+3+i];
		if( osdata != 0) {
			ret = ZFORCE_ERROR;
			printf("Open LED#: ");
			for(j = 0; j < 8; j++) {
				if(osdata & (0x80>>j)) {
					printf("%d, ", j+1+(i<<3));
				}
			}
			printf("\n");
		}
	}

	res_length = res_buff[LED_SHORT_TAG_INDEX+1] - 1;
	for (i = 0; i < res_length; i++) {
		osdata = res_buff[LED_SHORT_TAG_INDEX+3+i];
		if( osdata != 0) {
			ret = ZFORCE_ERROR;
			printf("Short LED#: ");
			for(j = 0; j < 8; j++) {
				if(osdata & (0x80>>j)) {
					printf("%d, ", j+1+(i<<3));
				}
			}
			printf("\n");
		}
	}

	res_length = res_buff[PD_SHORT_TAG_INDEX+1] - 1;
	for (i = 0; i < res_length; i++) {
		osdata = res_buff[PD_SHORT_TAG_INDEX+3+i];
		if( osdata != 0) {
			ret = ZFORCE_ERROR;
			printf("Short PD#: ");
			for(j = 0; j < 8; j++) {
				if(osdata & (0x80>>j)) {
					printf("%d, ", j+1+(i<<3));
				}
			}
			printf("\n");
		}
	}

	if(res_buff != NULL)
		free(res_buff);

	return ret;

}

U_BOOT_CMD(
        st_zforce, 1,      1,      do_st_zforce,
        "st touch zforce opensort test",
        ""
);

extern void setup_touch_pin(void);

static int do_zforce(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;

	if(identify_touch() != 0) {
		printf("%s unable to identify MCU touch\n", __FUNCTION__);
		printf("Open/Short CMD failed\n");
		return ZFORCE_ERROR;
	}
	udelay(500000);

	//printf("Zforce Open/Short test: ");

	if( supported_touch[installed_touch].id == ST_MCU ) {
		printf("Start ST touch testing...\n");
		ret = do_st_zforce(NULL,0,0,NULL);
	} else if( supported_touch[installed_touch].id == TI_MCU ) {
		printf("Start TI touch testing...\n");
		ret = do_ti_zforce(NULL,0,0,NULL);
	}

	printf("\nZforce Open/Short test: ");
	if (ret)
		printf("FAIL\n");
	else
		printf("PASS\n");

	return 0;
}

U_BOOT_CMD(
        zforce, 1,0,do_zforce,
        "zforce -show failed debug info",
        ""
);


static int do_cmd_zforce(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char *cmd = NULL;
	int i;
	unsigned char *res_buff = NULL;
	int res_length;
	int ret;

	if (installed_touch == TOUCH_NUMBER) {
		if (identify_touch() != 0) {
			printf("No supported MCU touch\n");
			return ZFORCE_ERROR;
		}
	}

	cmd = malloc(argc-1);
	if(NULL == cmd)
	{
		printf("Memmory malloc error\n");
		return ZFORCE_ERROR;
	}

	for(i=0; i<argc-1; i++)
	{
		cmd[i] = (unsigned char)simple_strtol(argv[i+1],NULL,16);
	}

	if( argc > 1 ){
		if( CMD_LEAD_BYTE != cmd[0] )
		{
			printf("Command should with leading byte:0x%2X\n",CMD_LEAD_BYTE);
			return ZFORCE_ERROR;
		}
	}

	zforce_print_msg((unsigned char *)"Request", cmd, (argc - 1));
	ret = zforce_cmd(cmd,argc-1,&res_buff,&res_length);
	if(ret){
		printf("%s Command ERROR\n", __FUNCTION__);
		return ZFORCE_ERROR;
	}
	zforce_print_msg((unsigned char *)"Response", res_buff, res_length);
	
	if (cmd != NULL)
		free(cmd);
	if (res_buff != NULL)
		free(res_buff);

	return ZFORCE_PASS;
}

U_BOOT_CMD(
        cmd_zforce, CONFIG_SYS_MAXARGS,      1,      do_cmd_zforce,
        "zforce command testing",
        ""
);

static int do_st_zforce_level(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return 0;
}

static int do_ti_zforce_level(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char cmd_init[3]={0xEE,0x01,0x01};
	unsigned char cmd_ledlevel_x[4]={0xEE,0x02,0x20,0x00};
	unsigned char cmd_ledlevel_y[4]={0xEE,0x02,0x20,0x01};
	int res_length, led_number;
	unsigned char *res_buff,leddata,pddata0,pddata1, ledname[3];
	int ret_x=0, ret_y = 0;
	int i, j;

	ret_x = zforce_cmd(cmd_init,3,&res_buff,&res_length);
	if(ret_x) {
		printf("%s Init cmd failed\n",__FUNCTION__);
		goto EXIT_ERROR;
	}
	
	if (res_buff != NULL)
		free(res_buff);

	ret_x = zforce_cmd(cmd_ledlevel_x,4,&res_buff,&res_length);
	if(ret_x) {
		printf("%s X_Axis led level cmd error\n", __FUNCTION__);
		goto EXIT_ERROR;
	}
	
	zforce_print_msg((unsigned char *)"Get X_Axis data respeonse:",res_buff,res_length);

	if((res_buff[0] != 0x1C) || (res_buff[1] != 0)) {
		printf("X_Axis led level wrong response.\n");
		ret_x = ZFORCE_ERROR;
	}
	
	if (res_buff != NULL)
		free(res_buff);
	
	ret_y = zforce_cmd(cmd_ledlevel_y,4,&res_buff,&res_length);
	if(ret_y) {
		printf("%s Y_Axis led level cmd error\n", __FUNCTION__);
		goto EXIT_ERROR;
	}
	
	zforce_print_msg((unsigned char *)"Get Y_Axis data respeonse:",res_buff, res_length);
	
	if((res_buff[0] != 0x1C) || (res_buff[1] != 1)) {
		printf("Y_Axis led level wrong response.\n");
		ret_y = ZFORCE_ERROR;
	}
	
	if (res_buff != NULL)
		free(res_buff);
	if(ret_x || ret_y)
		return ZFORCE_ERROR;
	else
		return ZFORCE_PASS;	
EXIT_ERROR:
	if (res_buff != NULL)
		free(res_buff);
	return ZFORCE_ERROR;
}

static int do_zforce_level(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;

	if(identify_touch() != 0) {
		printf("%s unable to identify MCU touch\n", __FUNCTION__);
		printf("Open/Short CMD failed\n");
		return ZFORCE_ERROR;
	}
	udelay(500000);

	//printf("Zforce LED level: ");

	if( supported_touch[installed_touch].id == ST_MCU ) {
		printf("Start ST LED level testing...\n");
		ret = do_st_zforce_level(NULL,0,0,NULL);
	} else if( supported_touch[installed_touch].id == TI_MCU ) {
		printf("Start TI LED level testing...\n");
		ret = do_ti_zforce_level(NULL,0,0,NULL);
	}

	return 0;
}

U_BOOT_CMD(
        zforce_level, 1,0,do_zforce_level,
        "zforce_level -led level",
        ""
);

static int do_st_zforce_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return 0;
}

static int do_ti_zforce_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char cmd_init[3]={0xEE,0x01,0x01};
	unsigned char cmd_statusrequest[3]={0xEE,0x01,0x1E};
	int res_length;
	unsigned char *res_buff;
	int ret=0;
	int i;
	int rmajor,rminor,rbuild,rrevision;

	ret = zforce_cmd(cmd_init,3,&res_buff,&res_length);
	if(ret) {
		printf("%s Init cmd failed\n",__FUNCTION__);
		goto EXIT_ERROR;
	}
	
	if (res_buff != NULL)
		free(res_buff);

	ret = zforce_cmd(cmd_statusrequest,3,&res_buff,&res_length);
	if(ret) {
		printf("%s status request cmd error\n", __FUNCTION__);
		goto EXIT_ERROR;
	}
	
	//zforce_print_msg((unsigned char *)"Status respeonse:",res_buff,res_length);

	if(res_buff[0] != 0x1E) {
		printf("Status request wrong response.\n");
		ret = ZFORCE_ERROR;
	}

	i = 1;
	rmajor = res_buff[i++] | (res_buff[i++]<<8);
	rminor = res_buff[i++] | (res_buff[i++]<<8);
	rbuild = res_buff[i++] | (res_buff[i++]<<8);
	rrevision = res_buff[i++] | (res_buff[i++]<<8);

	printf("Firmware version: %d.%d.b%d.r%d\n", rmajor,rminor,rbuild,rrevision);
		
	if (res_buff != NULL)
		free(res_buff);

	if(ret)
		return ZFORCE_ERROR;
	else
		return ZFORCE_PASS;
	
EXIT_ERROR:
	if (res_buff != NULL)
		free(res_buff);
	return ZFORCE_ERROR;
}

static int do_zforce_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;

	if(identify_touch() != 0) {
		printf("%s unable to identify MCU touch\n", __FUNCTION__);
		printf("Open/Short CMD failed\n");
		return ZFORCE_ERROR;
	}
	udelay(500000);

	//printf("Zforce LED level: ");

	if( supported_touch[installed_touch].id == ST_MCU ) {
		printf("Get ST FW version...\n");
		ret = do_st_zforce_version(NULL,0,0,NULL);
	} else if( supported_touch[installed_touch].id == TI_MCU ) {
		printf("Get TI FW version...\n");
		ret = do_ti_zforce_version(NULL,0,0,NULL);
	}

	return 0;
}
U_BOOT_CMD(
        zforce_version, 1,0,do_zforce_version,
        "zforce_version -get fw version",
        ""
);

