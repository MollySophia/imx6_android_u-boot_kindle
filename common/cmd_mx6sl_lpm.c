#include <common.h>
#include <command.h>
#include <environment.h>
#include <fsl-mx6sl-lpm.h>

#ifdef CONFIG_FSL_MX6SL_LPM_TEST
extern int test_entry(unsigned long action_id);
#endif

#if 0
extern u8* _low_power_code_start;
u8* gsource = 0x900000;
u8* gdest = NULL;
extern u32 _low_power_size;
void init_low_power_code(void)
{
	u32 i = 0;
/*
	gsource = _low_power_code_start;
	gdest = 0x920000;
	for(i = 0; i < _low_power_size; i++) {
		gdest[i] = gsource[i];
	}
*/
}
#endif

extern void enable_pm_epd(void);
extern void disable_pm_epd(void);

int do_low_power(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    unsigned long test_id;
    if (argc < 2) {
	cmd_usage(cmdtp);    
        return 1;
    }
    test_id = simple_strtoul(argv[1], NULL, 16);
#ifdef CONFIG_FSL_MX6SL_LPM_TEST
	switch(test_id)
	{
            case TEST_ID_IDLE_MODE:
	   	printf("Entering Idle mode...\n");
		break;
            case TEST_ID_SUSPEND_MODE:
	        printf("Entering Suspend mode...\n");
		break;
            case TEST_ID_SHIPPING_MODE:
	   	printf("Entering Shipping mode...\n");
		break;
	    default:
	   	cmd_usage(cmdtp);
	        return 0;
	}
    disable_pm_epd();
    test_entry(test_id);
#endif    
    return 0;
}

U_BOOT_CMD(
	lpm, 2, 1, do_low_power,
	"Exercise Low Power modes in iMX6SL",
	"lpm <test number> :options are 1 2 or 3 - 1=Enter Idle mode(ARM WFI), 2=Enter Suspend mode, 3=Enter Ship mode\n");

