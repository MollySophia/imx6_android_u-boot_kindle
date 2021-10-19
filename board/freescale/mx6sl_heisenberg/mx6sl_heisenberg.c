/*
 * Copyright (C) 2013-2014 Freescale Semiconductor, Inc.
 * Copyright (C) 2015 Amazon.com, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <netdev.h>
#include <lab126/lab126_boards_info.h>
#ifdef CONFIG_SYS_I2C_MXC
#include <i2c.h>
#include <asm/imx-common/mxc_i2c.h>
#endif
#if defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc_fb.h>
#if defined(CONFIG_WAVEFORM_BUILTIN)
#include "epdc_waveform_builtin.c"
#endif /* CONFIG_WAVEFORM_BUILTIN */
#if defined(CONFIG_BMP_SPLASH_BUILTIN)
#include "bmp_logo_builtin.c"
#endif /* CONFIG_BMP_SPLASH_BUILTIN */
#endif /* CONFIG_MXC_EPDC */
#include "pinmux.h"

#ifdef CONFIG_FASTBOOT
#include <lab126/lab126_partitions_info.h>
#include <fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FASTBOOT*/

#if defined (CONFIG_CMD_IDME)

#include <lab126/idme.h>
static const struct board_type *get_board_type(void);
#endif

#include "led_control.h"

#include <lab126/pmic.h>


DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_IDME
#define TATOO_CODE_START_OFFSET 3
extern int g_cur_mmc;

int setup_board_info(void);
#endif

#ifdef CONFIG_MFGTOOL
static char boardid_input_buf[256];
extern int readline_into_buffer (const char *const prompt, char * buffer, int timeout);
#endif

#define BOOT_MODE_UNKNOWN -1

#ifdef CONFIG_IRAM_BOOT
/* should be called after RAM init and POST */
unsigned int *mr5_bist = (unsigned int *)0x80800000; //use kernel load address to transfer to bist
unsigned int *mr6_bist = (unsigned int *)0x80800004;
u8 get_ddr_mfginfo(void)
{
        u32 mapsr, madpcr0, mdscr, mdmrr;
        u8 mfgid = 0x0;
	u32 mr5,mr6;

#if 0
        const char *rev = (const char *) get_board_id16();
#endif

#define MMDC0_MAPSR (MMDC_P0_BASE_ADDR + 0x0404)
#define MMDC0_MADPCR0 (MMDC_P0_BASE_ADDR + 0x0410)
#define MMDC0_MDSCR (MMDC_P0_BASE_ADDR + 0x001C)
#define MMDC0_MDMRR (MMDC_P0_BASE_ADDR + 0x0034)

        // save registers
        mapsr = __raw_readl(MMDC0_MAPSR);
        madpcr0 = __raw_readl(MMDC0_MADPCR0);
        mdscr = __raw_readl(MMDC0_MDSCR);

        //disable MMDC automatic power savings
        __raw_writel(0x1, MMDC0_MAPSR);
        //set SBS_EN bit
        __raw_writel(0x100, MMDC0_MADPCR0);
        //set CON_REQ bit
        __raw_writel(0x8000, MMDC0_MDSCR);

        //wait for CON_ACK!
        while(!(__raw_readl(MMDC0_MDSCR) & 0x4000)) ;

        //allow IP access, precharge all
        __raw_writel(0x00008050, MMDC0_MDSCR);
        //set MRR cmd and addr as MR5
        __raw_writel(0x00058060, MMDC0_MDSCR);

        //poll for read data valid
        while(!(__raw_readl(MMDC0_MDSCR) & 0x400)) ;

        mdmrr = __raw_readl(MMDC0_MDMRR);

	mr5 = mdmrr & 0xFF;
	mfgid = mdmrr & 0xFF;

        //set MRR cmd and addr as MR6
        __raw_writel(0x00068060, MMDC0_MDSCR);

        //poll for read data valid
        while(!(__raw_readl(MMDC0_MDSCR) & 0x400)) ;

        mdmrr = __raw_readl(MMDC0_MDMRR);
	
	mr6 = mdmrr & 0xFF;
#if 0
        if(BOARD_IS_abc123(rev)) {
                mfgid = mdmrr & 0xFF;
        } else {
                /* On abc123 and abc123--
                   swap bits according to dataline mapping
                   --lower 16-bits map(which we care)--
                   |  SoC    |  LPDDR2 |
                   | D0-D15  |  D15-D0 | */

                //reverse bits - from 'Bit Twiddling Hacks'
                u16 r, v;
                r = v = mdmrr & 0xFFFF;
                int s = (sizeof(v) * 8) - 1; // extra shift needed at end

                for (v >>= 1; v; v >>= 1)
                {
                        r <<= 1; r |= v & 1; s--;
                }
                r <<= s; // shift remaining when highest bits are zero

                //get just LSB which has mfgid
                mfgid = r & 0xFF;
        }
#endif
        //restore
        __raw_writel(mapsr, MMDC0_MAPSR);
        __raw_writel(madpcr0, MMDC0_MADPCR0);
        __raw_writel(mdscr, MMDC0_MDSCR);
	
	printf("MR5=0x%x,MR6=0x%x ",mr5,mr6);
	
	*mr5_bist = mr5;
	*mr6_bist = mr6;
        return mfgid;
}
#endif /*ifdef CONFIG_IRAM_BOOT */

int dram_init(void)
{
        const struct board_type *board;
#ifdef CONFIG_IRAM_BOOT
	get_ddr_mfginfo();
#endif


	while(1)
        {		
		gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);
#if CONFIG_CMD_IDME
		board = get_board_type();

	    	if (board != NULL) 
                {
#ifdef CONFIG_MFGTOOL
			gd->ram_size = PHYS_SDRAM_SIZE;
#else
			gd->ram_size = board->mem_size;
#endif

#ifdef CONFIG_IRAM_BOOT
	      		switch (board->mem_type) 
			{
	        		case MEMORY_TYPE_LPDDR2:
          	    		if (board->mem_size == MEMORY_SIZE_256MB || board->mem_size == MEMORY_SIZE_512MB) 
				{
                        		//lpddr2_init(board);//TODO: Need to add this function back from abc123 platform. just in case Lab126 support diff size/settings of DDR
		    		} 
				else 
				{
		        		printf("Error! invalid memory config!\n");
		    		}
		    		break;
	        		default:
		    			printf("Error! unsupported memory type!\n");
	    		}
#endif
			return 0;
                }
#endif
#ifdef CONFIG_MFGTOOL
		
                /* Clear out buffer */
                memset(boardid_input_buf, 0, sizeof(boardid_input_buf));

                printf("Board ID is invalid!  Please enter a valid board id:\n");
                readline_into_buffer(">", boardid_input_buf, 0);

                if (strlen(boardid_input_buf) != BOARD_PCBA_LEN) 
		{
                        printf("\nError! Board ID must be %d chars long.\n\n", BOARD_PCBA_LEN);
                        continue;
                }

                idme_update_var("pcbsn", boardid_input_buf); 
                /* Set bootmode to diags if this is the first boot */
		           
#else
		printf("\nInvalid board id!  Can't determine system type for RAM init.. bailing!\n");
				
#endif
		idme_update_var("bootmode", "uboot");  //Don't allow to boot to kernel if the pcbsn is not set
		return 0;
	}
	return 0;
}



#ifdef CONFIG_FSL_ESDHC

static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC1_BASE_ADDR, 0, 4},
	{USDHC2_BASE_ADDR, 0, 8},
	{USDHC3_BASE_ADDR, 0, 4},
};

int mmc_get_env_devno(void)
{
	u32 soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	u32 dev_no;
	u32 bootsel;

	bootsel = (soc_sbmr & 0x000000FF) >> 6 ;

	/* If not boot from sd/mmc, use default value */
	if (bootsel != 1)
		return CONFIG_SYS_MMC_ENV_DEV;

	/* BOOT_CFG2[3] and BOOT_CFG2[4] */
	dev_no = (soc_sbmr & 0x00001800) >> 11;

	return dev_no;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;	/* aloway present for Eanab wifi*/
		break;
	case USDHC2_BASE_ADDR:
		ret = 1;	/* always present: emmc */
		break;
	case USDHC3_BASE_ADDR:
		ret = 1;	/* always present: wifi */
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int i;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 * mmc2                    USDHC3
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			setup_iomux_usdhc1();
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 1:
			setup_iomux_usdhc2();
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 2:
			setup_iomux_usdhc3();
			usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return 0;
			}

			if (fsl_esdhc_initialize(bis, &usdhc_cfg[i]))
				printf("Warning: failed to initialize mmc dev %d\n", i);
	}
#ifdef CONFIG_CMD_IDME
	setup_board_info();
#endif
	return 0;
}

int check_mmc_autodetect(void)
{
	char *autodetect_str = getenv("mmcautodetect");

	if ((autodetect_str != NULL) &&
		(strcmp(autodetect_str, "yes") == 0)) {
		return 1;
	}

	return 0;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_devno();

	if (!check_mmc_autodetect())
		return;

	setenv_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw",
		mmc_map_to_kernel_blk(dev_no));
	setenv("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}
#endif

#ifdef CONFIG_PMIC_FITIPOWER
void fitipower_epdc_power(int enable)
{
        gpio_direction_output( IMX_GPIO_NR(3, 25) , enable);
        gpio_direction_output( IMX_GPIO_NR(3, 27) , enable);
}
#endif

#ifdef CONFIG_MXC_EPDC
#ifdef CONFIG_SPLASH_SCREEN
extern int mmc_get_env_devno(void);
int setup_splash_img(void)
{
#ifdef CONFIG_SPLASH_IS_IN_MMC
	int mmc_dev = mmc_get_env_devno();
	ulong offset = CONFIG_SPLASH_IMG_OFFSET;
	ulong size = CONFIG_SPLASH_IMG_SIZE;
	ulong addr = 0;
	char *s = NULL;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	s = getenv("splashimage");

	if (NULL == s) {
		puts("env splashimage not found!\n");
		return -1;
	}
	addr = simple_strtoul(s, NULL, 16);

	if (!mmc) {
		printf("MMC Device %d not found\n", mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
				      blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;
#endif

	return 0;
}
#endif

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 800,
	.vl_row = 600,
	.vl_pixclock = 26666667,
	.vl_left_margin = 8,
	.vl_right_margin = 100,
	.vl_upper_margin = 4,
	.vl_lower_margin = 8,
	.vl_hsync = 4,
	.vl_vsync = 1,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	.cmap = 0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 419,
	.gdsp_offs = 20,
	.gdoe_offs = 0,
	.gdclk_offs = 5,
	.num_ce = 1,
};

static void setup_epdc_power(void)
{
	/* Setup epdc voltage */

	/* EPDC_PWRSTAT - GPIO2[13] for PWR_GOOD status */
	gpio_direction_output( IMX_GPIO_NR(3, 25) , 1);
       
	//setting vcom voltage here?
	
        gpio_direction_output( IMX_GPIO_NR(3, 27) , 1);

}

struct wfm_header {
	unsigned int checksum:32;
	unsigned int file_length:32;
	unsigned int serial_number:32;
	unsigned int run_type:8;
	unsigned int fpl_platform:8;
	unsigned int fpl_lot:16;
	unsigned int mode_version:8;
	unsigned int wf_version:8;
	unsigned int wf_subversion:8;
	unsigned int wf_type:8;
	unsigned int panel_size:8;
	unsigned int amepd_part_number:8;
	unsigned int wf_revision:8;
	unsigned int frame_rate:8;
	unsigned int reserved1_0:8;
	unsigned int vcom_shifted:8;
	unsigned int reserved1_1:16;

	unsigned int xwia:24;
	unsigned int cs1:8;

	unsigned int wmta:24;
	unsigned int fvsn:8;
	unsigned int luts:8;
	unsigned int mc:8;
	unsigned int trc:8;
	unsigned int advanced_wfm_flags:8;
	unsigned int eb:8;
	unsigned int sb:8;
	unsigned int reserved0_1:8;
	unsigned int reserved0_2:8;
	unsigned int reserved0_3:8;
	unsigned int reserved0_4:8;
	unsigned int reserved0_5:8;
	unsigned int cs2:8;
};

int setup_waveform_file(void)
{
#ifdef CONFIG_WAVEFORM_FILE_IN_MMC
	int mmc_dev = mmc_get_env_devno();
	ulong offset = CONFIG_WAVEFORM_FILE_OFFSET;
	ulong size = CONFIG_WAVEFORM_FILE_SIZE;
	ulong addr = CONFIG_WAVEFORM_BUF_ADDR;
	struct mmc *mmc = find_mmc_device(mmc_dev);
	uint blk_start, blk_cnt, n;

	if (!mmc) {
		printf("MMC Device %d not found\n", mmc_dev);
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	blk_start = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
	blk_cnt = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
	n = mmc->block_dev.block_read(mmc_dev, blk_start,
				      blk_cnt, (u_char *)addr);
	flush_cache((ulong)addr, blk_cnt * mmc->read_bl_len);

	return (n == blk_cnt) ? 0 : -1;

#elif defined(CONFIG_WAVEFORM_BUILTIN)
	void *addr = (void *)((uintptr_t)panel_info.epdc_data.waveform_buf_addr);
	ulong len = epdc_builtin_fw_gz_len;
	int wv_data_offs;
	void *scratch_buf = (void *)((uintptr_t)CONFIG_SCRATCH_BUF_ADDR);
	struct wfm_header *hdr = (struct wfm_header *)((uintptr_t)CONFIG_SCRATCH_BUF_ADDR);

	if (gunzip(scratch_buf, CONFIG_WAVEFORM_FILE_SIZE,
		   (uchar *)epdc_builtin_fw_gz, &len) != 0) {
	  printf("Error - gunzip faield\n");
	  return -1;
	}

	/* IMPORTANT. waveform data starts from some offset. */
	wv_data_offs = sizeof(struct wfm_header) + hdr->trc + 2;
	scratch_buf += wv_data_offs;
	memcpy(addr, scratch_buf, len - wv_data_offs);

	return 0;
#else
	return -1;
#endif
}

static void setup_epdc(void)
{
	unsigned int reg;
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/*** epdc Maxim PMIC settings ***/

	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk from PFD_400M, set to 396/2 = 198MHz */
	reg = readl(&ccm_regs->chsccdr);
	reg &= ~0x3F000;
	reg |= (0x4 << 15) | (1 << 12);
	writel(reg, &ccm_regs->chsccdr);

	/* EPDC AXI clk enable */
	reg = readl(&ccm_regs->CCGR3);
	reg |= 0x0030;
	writel(reg, &ccm_regs->CCGR3);

	/* EPDC PIX clk from PFD_540M, set to 540/4/5 = 27MHz */
	reg = readl(&ccm_regs->cscdr2);
	reg &= ~0x03F000;
	reg |= (0x5 << 15) | (4 << 12);
	writel(reg, &ccm_regs->cscdr2);

	reg = readl(&ccm_regs->cbcmr);
	reg &= ~0x03800000;
	reg |= (0x3 << 23);
	writel(reg, &ccm_regs->cbcmr);

	/* EPDC PIX clk enable */
	reg = readl(&ccm_regs->CCGR3);
	reg |= 0x0C00;
	writel(reg, &ccm_regs->CCGR3);

	/* Assign working buf and waveform buf. Both should be 8 bytes aligned */
	/* if stride is configured, working buf does not need to be 8 bytes aligned.
	 * However, it's better to keep it aligned for better performance.
	 */
	panel_info.epdc_data.working_buf_addr = CONFIG_WORKING_BUF_ADDR;
	panel_info.epdc_data.waveform_buf_addr = CONFIG_WAVEFORM_BUF_ADDR;

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;

	setup_epdc_power();

	/* Assign fb_base. Make sure it's 8 bytes aligned */
	gd->fb_base = CONFIG_FB_BASE;
}

void epdc_power_on(void)
{
	unsigned int reg;
	struct gpio_regs *gpio_regs = (struct gpio_regs *)GPIO2_BASE_ADDR;

	fitipower_epdc_power(1);

	udelay(500);
}

void epdc_power_off(void)
{
	fitipower_epdc_power(0);
}
#endif

#ifdef CONFIG_SYS_I2C_MXC
/* set all switches APS in normal and PFM mode in standby */
static int setup_pmic_mode(int chip)
{

	return 0;
}

static int setup_pmic_voltages(void)
{
	volatile unsigned char value;
	int ret = 0;

	ret = i2c_set_bus_num(0);
	
	value = 0x14;
	ret = i2c_write(0x4b, 0x18, 1,(unsigned char *)&value, 1); //ldo5 set to 1.8v
	ret = i2c_write(0x4b, 0x19, 1,(unsigned char *)&value, 1);

        value = 0x30;
        ret = i2c_write(0x4b, 0x17, 1, (unsigned char *)&value, 1); //ldo4 set to 3.2v
	ret = i2c_write(0x4b, 0x14, 1, (unsigned char *)&value, 1);//ldo1 = 3.2V

	udelay(100000); //wait 3.3V io stable
	value = 0x44;
	ret = i2c_write(0x4b, 0x12, 1, (unsigned char *)&value, 1);//Enable LDO4, LDO5
	if (ret)
    	{
        	printf("\n%s:failed to setup pmic voltages \n", __func__);
        	return 0;
	}	
	
	udelay(100000);
	return 1;
}

#ifdef CONFIG_LDO_BYPASS_CHECK
void ldo_mode_set(int ldo_bypass)
{
	unsigned char value;
	int is_400M;

	/* swith to ldo_bypass mode */
	if (ldo_bypass) {
		prep_anatop_bypass();

		/* decrease VDDARM to 1.1V */
		if (i2c_read(0x8, 0x20, 1, &value, 1)) {
			printf("Read SW1AB error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x20;
		if (i2c_write(0x8, 0x20, 1, &value, 1)) {
			printf("Set SW1AB error!\n");
			return;
		}
		/* increase VDDSOC to 1.3V */
		if (i2c_read(0x8, 0x2e, 1, &value, 1)) {
			printf("Read SW1C error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x28;
		if (i2c_write(0x8, 0x2e, 1, &value, 1)) {
			printf("Set SW1C error!\n");
			return;
		}

		is_400M = set_anatop_bypass(0);

		/*
		 * MX6SL: VDDARM:1.175V@800M; VDDSOC:1.175V@800M
		 *        VDDARM:0.975V@400M; VDDSOC:1.175V@400M
		 */
		if (i2c_read(0x8, 0x20, 1, &value, 1)) {
			printf("Read SW1AB error!\n");
			return;
		}
		value &= ~0x3f;
		if (is_400M)
			value |= 0x1b;
		else
			value |= 0x23;
		if (i2c_write(0x8, 0x20, 1, &value, 1)) {
			printf("Set SW1AB error!\n");
			return;
		}

		/* decrease VDDSOC to 1.175V */
		if (i2c_read(0x8, 0x2e, 1, &value, 1)) {
			printf("Read SW1C error!\n");
			return;
		}
		value &= ~0x3f;
		value |= 0x23;
		if (i2c_write(0x8, 0x2e, 1, &value, 1)) {
			printf("Set SW1C error!\n");
			return;
		}

		finish_anatop_bypass();
		printf("switch to ldo_bypass mode!\n");
	}

}
#endif
#endif

int board_early_init_f(void)
{
	setup_iomux_uart1();

	return 0;
}

#ifdef CONFIG_CMD_IDME
int board_info_valid (u8 *info, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if ((info[i] < '0') &&
            (info[i] > '9') &&
            (info[i] < 'A') &&
            (info[i] > 'Z'))
            return 0;
    }

    return 1;
}

int setup_board_info(void)
{
#if defined(CONFIG_CMD_IDME)
        int do_clear_old_idme = idme_check_update();

#ifdef CONFIG_QBOOT
        {
                char bootmode[BOARD_BOOTMODE_LEN + 1];
                if (!idme_get_var("bootmode", bootmode, sizeof(bootmode)) &&
                        !strncmp(bootmode, "qboot", 5)) {
                        puts("QBOOT\n");
                        gd->flags |= GD_FLG_QUICKBOOT;

                        // userland will continue using the updated area, don't clear it.
                        do_clear_old_idme = 0;
                } else {
                        gd->flags &= ~GD_FLG_QUICKBOOT;
                }
        }
#endif
        if (do_clear_old_idme) 
		idme_clear_update();

        if (idme_get_var("pcbsn", (char *) gd->board_id, sizeof(gd->board_id)))
#endif
        {
                /* not found: clean up garbage characters. */
                memset((char *) gd->board_id, 0, sizeof(gd->board_id));
        }
#if defined(CONFIG_CMD_IDME)
        if (idme_get_var("serial", (char *) gd->serial_number, sizeof(gd->serial_number)))
#endif
        {
                /* not found: clean up garbage characters. */
                memset((char *)gd->serial_number, 0, sizeof(gd->serial_number));
        }

#if defined(CONFIG_CMD_IDME)
	if (idme_get_var("sec", (char *) gd->board_sec, sizeof(gd->board_sec)))
#endif
	{
               /* not found: clean up garbage characters. */
               memset((char *)gd->board_sec, 0, sizeof(gd->board_sec));
	}

	return 0;
}

#endif

/*************************************************************************
 * get_board_serial() - setup to pass kernel serial number information
 *      return: alphanumeric containing the serial number.
 *************************************************************************/
const u8 *get_board_serial(void)
{
#ifdef CONFIG_CMD_IDME
	if (!board_info_valid((u8 *)gd->serial_number, BOARD_DSN_LEN))
           return (u8 *) "0000000000000000";
    	else
           return (u8 *)gd->serial_number;
#else
	return (u8 *) "0480100000000000";
#endif
}

#ifdef CONFIG_SERIAL_IDME
/* This function is a wrapper around get_board_serial(void) as this function is defined
 * differently in asm/bootm.h.
 */
const u8 *get_board_serial_idme(void) {
	return get_board_serial();
}
#endif

/*************************************************************************
 * get_board_id16() - setup to pass kernel board revision information
 *      16-byte alphanumeric containing the board revision.
 *************************************************************************/
const u8 *get_board_id16(void)
{
#ifdef CONFIG_CMD_IDME
	if (!board_info_valid((u8 *)gd->board_id, BOARD_PCBA_LEN))
            return (u8 *) "0000000000000000";
    	else
            return (u8 *)gd->board_id;
#else
	return (u8 *) "019110000000000000";
#endif
}

/*
 * is_platform_board() - check if running device is platform board or not
 */
int is_platform_board(void)
{
	return gd->board_sec[0] == 0;
}

static void disable_watchdog_power_count_down_event(void)
{
    __raw_writew(~0x1, (WDOG1_BASE_ADDR+0x08));
}


#define I2C1_PAD_CTRL   (PAD_CTL_LVE | PAD_CTL_HYS | PAD_CTL_ODE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C2_PAD_CTRL  ( PAD_CTL_HYS | PAD_CTL_ODE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_48ohm )

#define I2C3_PAD_CTRL   (PAD_CTL_HYS | PAD_CTL_ODE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_48ohm )

/* I2C: I2C1 */
struct i2c_pads_info i2c1_info = {
	.sda = {
		.i2c_mode = MX6_PAD_I2C1_SDA__I2C1_SDA | MUX_PAD_CTRL(I2C1_PAD_CTRL),
		.gpio_mode = MX6_PAD_I2C1_SDA__GPIO_3_13 | MUX_PAD_CTRL(I2C1_PAD_CTRL),
		.gp = IMX_GPIO_NR(3, 13),
	},
	.scl = {
		.i2c_mode = MX6_PAD_I2C1_SCL__I2C1_SCL | MUX_PAD_CTRL(I2C1_PAD_CTRL),
		.gpio_mode = MX6_PAD_I2C1_SCL__GPIO_3_12 | MUX_PAD_CTRL(I2C1_PAD_CTRL),
		.gp = IMX_GPIO_NR(3, 12),
	},
};
struct i2c_pads_info i2c2_info = {
	.sda = {
		.i2c_mode = MX6_PAD_I2C2_SDA__I2C2_SDA | MUX_PAD_CTRL(I2C2_PAD_CTRL),
		.gpio_mode = MX6_PAD_I2C2_SDA__GPIO_3_15 | MUX_PAD_CTRL(I2C2_PAD_CTRL),
		.gp = IMX_GPIO_NR(3, 15),
	},
	.scl = {
		.i2c_mode = MX6_PAD_I2C2_SCL__I2C2_SCL | MUX_PAD_CTRL(I2C2_PAD_CTRL),
		.gpio_mode = MX6_PAD_I2C2_SCL__GPIO_3_14 | MUX_PAD_CTRL(I2C2_PAD_CTRL),
		.gp = IMX_GPIO_NR(3, 14),
	},
};
struct i2c_pads_info i2c3_info = {
	.sda = {
		.i2c_mode = MX6_PAD_EPDC_SDCE3__I2C3_SDA | MUX_PAD_CTRL(I2C3_PAD_CTRL),
		.gpio_mode = MX6_PAD_EPDC_SDCE3__GPIO_1_30 | MUX_PAD_CTRL(I2C3_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 30),
	},
	.scl = {
		.i2c_mode = MX6_PAD_EPDC_SDCE2__I2C3_SCL | MUX_PAD_CTRL(I2C3_PAD_CTRL),
		.gpio_mode = MX6_PAD_EPDC_SDCE2__GPIO_1_29 | MUX_PAD_CTRL(I2C3_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 29),
	},
};


static void set_unused_pins(void)
{
	//Set unused GPIOs to input
	gpio_direction_input(IMX_GPIO_NR(1,0));
	gpio_direction_input(IMX_GPIO_NR(1,1));
	gpio_direction_input(IMX_GPIO_NR(1,2));
	gpio_direction_input(IMX_GPIO_NR(1,3));
	gpio_direction_input(IMX_GPIO_NR(1,4));
	gpio_direction_input(IMX_GPIO_NR(1,5));
	gpio_direction_input(IMX_GPIO_NR(1,6));
	gpio_direction_input(IMX_GPIO_NR(1,15));
	gpio_direction_input(IMX_GPIO_NR(1,16));
	gpio_direction_input(IMX_GPIO_NR(1,17));
	gpio_direction_input(IMX_GPIO_NR(1,18));
	gpio_direction_input(IMX_GPIO_NR(1,19));
	gpio_direction_input(IMX_GPIO_NR(1,20));
	gpio_direction_input(IMX_GPIO_NR(1,21));
	gpio_direction_input(IMX_GPIO_NR(1,22));
	gpio_direction_input(IMX_GPIO_NR(1,26));
	gpio_direction_input(IMX_GPIO_NR(1,28));

        gpio_direction_input(IMX_GPIO_NR(2,1));
        gpio_direction_input(IMX_GPIO_NR(2,7));
        gpio_direction_input(IMX_GPIO_NR(2,8));
        gpio_direction_input(IMX_GPIO_NR(2,9));
        gpio_direction_input(IMX_GPIO_NR(2,10));
        gpio_direction_input(IMX_GPIO_NR(2,11));
        gpio_direction_input(IMX_GPIO_NR(2,12));
        gpio_direction_input(IMX_GPIO_NR(2,13));
        gpio_direction_input(IMX_GPIO_NR(2,14));
        gpio_direction_input(IMX_GPIO_NR(2,16));
        gpio_direction_input(IMX_GPIO_NR(2,17));
        gpio_direction_input(IMX_GPIO_NR(2,18));
        gpio_direction_input(IMX_GPIO_NR(2,21));
        gpio_direction_input(IMX_GPIO_NR(2,22));
        gpio_direction_input(IMX_GPIO_NR(2,23));
        gpio_direction_input(IMX_GPIO_NR(2,27));
        gpio_direction_input(IMX_GPIO_NR(2,28));
        gpio_direction_input(IMX_GPIO_NR(2,29));
        gpio_direction_input(IMX_GPIO_NR(2,30));
        gpio_direction_input(IMX_GPIO_NR(2,31));

        gpio_direction_input(IMX_GPIO_NR(3,0));
        gpio_direction_input(IMX_GPIO_NR(3,1));
        gpio_direction_input(IMX_GPIO_NR(3,2));
        gpio_direction_input(IMX_GPIO_NR(3,3));
        gpio_direction_input(IMX_GPIO_NR(3,8));
        gpio_direction_input(IMX_GPIO_NR(3,9));
        gpio_direction_input(IMX_GPIO_NR(3,10));
        gpio_direction_input(IMX_GPIO_NR(3,11));
        gpio_direction_input(IMX_GPIO_NR(3,19));
        gpio_direction_input(IMX_GPIO_NR(3,20));


        gpio_direction_input(IMX_GPIO_NR(4,18));
        gpio_direction_input(IMX_GPIO_NR(4,19));
        gpio_direction_input(IMX_GPIO_NR(4,22));
        gpio_direction_input(IMX_GPIO_NR(4,23));
        gpio_direction_input(IMX_GPIO_NR(4,25));
        gpio_direction_input(IMX_GPIO_NR(4,26));
	
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

    	/* set pmic voltages before iomux setup*/
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x48, &i2c1_info);
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x4B, &i2c1_info);
	i2c_set_bus_num(0);
	pmic_init();
	setup_pmic_mode(0);
	setup_pmic_voltages();

    	udelay(10000);

	setup_iomux();
	set_unused_pins();

	//some delay to stabilise
	udelay(2000);

	//disable WFIFI as default
        gpio_direction_output( IMX_GPIO_NR(2,15), 0);      //LCD_CLK
        gpio_direction_output( IMX_GPIO_NR(2,19), 0);      //LCD_RESET

	disable_watchdog_power_count_down_event();

#ifdef CONFIG_MXC_EPDC
	setup_epdc();
#endif
	return 0;
}

/* check if the recovery bit is set by kernel, it can be set by kernel
 * issue a command '# reboot fastboot' */
int fastboot_check_and_clean_flag(void)
{
#ifdef CONFIG_ABOOT_FLAG_PMIC
	u32 aboot_mode = board_pmic_get_aboot_mode();
	board_pmic_set_aboot_mode(ANDROID_BOOT_REGULAR_REBOOT);
#else
	u32	aboot_mode = imx_get_aboot_mode();
	imx_set_aboot_mode(ANDROID_BOOT_REGULAR_REBOOT);
#endif
	int ret = (aboot_mode == ANDROID_BOOT_FASTBOOT) ? 1:0;

#ifdef CONFIG_SWREV_SUPPORT
	ret |= run_command("swrev_img 0 0", 0);
#endif

#ifdef CONFIG_FASTBOOT_MAGIC_SEQ
	/* did we press fastboot combination keys */
	ret |= board_fastboot_keys_pressed();
#endif

	return ret;
}

int fastboot_set_flag_bootloader(void)
{
#ifdef CONFIG_ABOOT_FLAG_PMIC
	board_pmic_set_aboot_mode(ANDROID_BOOT_FASTBOOT);
#else
	imx_set_aboot_mode(ANDROID_BOOT_FASTBOOT);
#endif
	return 0;
}

static inline int check_boot_mode(void)
{
        char boot_mode[BOARD_BOOTMODE_LEN + 1];
#define NEW_BOOTCMD_LENGTH 21
        char boot_cmd[NEW_BOOTCMD_LENGTH];
	int rebootmode;

	/*
	 * The below code does not get used to boot into fastboot. On board_init,
	 * check_fastboot is invoked to check for fastboot mode using
	 * fastboot_check_and_clean_flag() and uses that to decide whether to boot
	 * into fastboot. This feature to set the bootcmd env to "bist" would be
	 * broken in the current state as it is since we do not have bist in e3os.
	 */
#ifdef CONFIG_ABOOT_FLAG_PMIC
	rebootmode = board_pmic_get_aboot_mode();
#else
	rebootmode = imx_get_aboot_mode();
#endif
	printf ("REBOOTMODE: %d\n", rebootmode);

	if(rebootmode == ANDROID_BOOT_FASTBOOT) {
		boot_cmd[0] = 0;
#ifdef CONFIG_BIST
		printf ("REBOOTMODE: BIST FASTBOOT\n");
		strcpy(boot_cmd, "fastboot");
#else
		printf ("REBOOTMODE: FASTBOOT\n");
		strcpy(boot_cmd, "bist");
#endif
		setenv("bootdelay", "0");
	} else {

#if defined(CONFIG_CMD_IDME)
        if (idme_get_var("bootmode", boot_mode, sizeof(boot_mode)))
#endif
        {
            return BOOT_MODE_UNKNOWN;
        }

        boot_cmd[0] = 0;

        if (!strncmp(boot_mode, "diags", 5)) {
            printf ("BOOTMODE OVERRIDE: DIAGS\n");
            /* clear bootargs to let the kernel choose them */
            strcpy(boot_cmd, "run diags_boot");
        } else if (!strncmp(boot_mode, "fastboot", 8)) {
            printf ("BOOTMODE OVERRIDE: FASTBOOT\n");
            strcpy(boot_cmd, "fastboot");
        } else if (!strncmp(boot_mode, "uboot", strlen("uboot"))) {
            printf ("BOOTMODE OVERRIDE: UBOOT\n");
            strcpy(boot_cmd, "bist");
        } else {
	    	printf("using default bootcmd");
	    	return BOOT_MODE_UNKNOWN;
		}
#ifdef CONFIG_BIST
				setenv("bootdelay", "-1");
#endif
	}

	setenv("bootcmd", boot_cmd);
	
	return 0;
}

void setup_touch_pin(void)
{
	gpio_direction_input( IMX_GPIO_NR(4, 3));
	gpio_direction_input( IMX_GPIO_NR(3, 24)); //bsl
	gpio_direction_output( IMX_GPIO_NR(4, 5) , 0);//reset
	udelay(100000);
	gpio_direction_output( IMX_GPIO_NR(4, 5) , 1);
}

void enable_pm_epd(void)
{
	gpio_direction_output(IMX_GPIO_NR(3,25),1);
	gpio_direction_output(IMX_GPIO_NR(3,27),1);
}

void disable_pm_epd(void)
{
	gpio_direction_output(IMX_GPIO_NR(3,27),0);
	gpio_direction_output(IMX_GPIO_NR(3,25),0);
}

int board_late_init(void)
{

#ifdef CONFIG_LP5523_LED_CONTROL
	/* Enable SOL animation */
	initialize_leds();
#endif

	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x50, &i2c2_info);//I2C1 need setup after touch voltage enabled
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x25, &i2c3_info);
	setup_touch_pin();
	
	disable_pm_epd();

	check_boot_mode();

	return 0;
}

u32 get_board_rev(void)
{
	return get_cpu_rev();
}

#ifdef CONFIG_FASTBOOT

void board_fastboot_setup(void)
{
	/* explicitly set MMC1 */
	setenv("fastboot_dev", "mmc"__stringify(CONFIG_MMC_DEFAULT_DEV));
}

void board_fastboot_load_partitions(void)
{
	struct fastboot_ptentry		ptable[CONFIG_NUM_PARTITIONS + 1];
	const struct partition_info_t	*part_info = partition_info_default;
	struct mmc			*mmc;
	block_dev_desc_t *dev_desc;
	int count = 0;
	int i;
	int mmc_no = CONFIG_MMC_DEFAULT_DEV;

	printf("flash target is MMC:%d\n", mmc_no);
	mmc = find_mmc_device(mmc_no);
	if (mmc && mmc_init(mmc))
		printf("MMC card init failed!\n");

	dev_desc = get_dev("mmc", mmc_no);
	if (NULL == dev_desc) {
		printf("** Block device MMC %d not supported\n", mmc_no);
		return;
	}

	memset((char *)ptable, 0, sizeof(ptable));
	/* fill gpt partition layout */
	for (i = 0; i < CONFIG_NUM_PARTITIONS; i++) {
		/* partition record */
		strcpy(ptable[count].name, part_info[i].name);
		ptable[count].start = part_info[i].address / dev_desc->blksz;
		if (part_info[i].size != PARTITION_FILL_SPACE)
			ptable[count].length = part_info[i].size / dev_desc->blksz;
		else
			ptable[count].length = dev_desc->lba - 128 - ptable[count].start;
		ptable[count].partition_id = part_info[i].partition;

		/* add to fastboot */
		fastboot_flash_add_ptn(&ptable[count]);
		/* next */
		++count;
	}

	/*check it out */
	//fastboot_flash_dump_ptn();

}

#ifdef CONFIG_ANDROID_RECOVERY
int check_recovery_cmd_file(void)
{
    return recovery_check_and_clean_flag();
}

void board_recovery_setup(void)
{
	int bootdev = get_boot_device();

	/*current uboot BSP only supports USDHC2*/
	switch (bootdev) {
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD1_BOOT:
	case MMC1_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
					"booti mmc0 recovery");
		break;
	case SD2_BOOT:
	case MMC2_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
					"booti mmc1 recovery");
		break;
	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
					"booti mmc2 recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("Unsupported bootup device for recovery: dev: %d\n",
			bootdev);
		return;
	}

	printf("setup env for recovery..\n");
	setenv("bootcmd", "run bootcmd_android_recovery");
}

#endif /*CONFIG_ANDROID_RECOVERY*/

#endif /*CONFIG_FASTBOOT*/


#ifdef CONFIG_MXC_KPD
#define MX6SL_KEYPAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PKE | PAD_CTL_PUE | \
				PAD_CTL_PUS_100K_UP | PAD_CTL_DSE_120ohm)

iomux_v3_cfg_t const mxc_kpd_pads[] = {
	(MX6_PAD_KEY_COL0__KPP_COL_0 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_KEY_COL1__KPP_COL_1 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_KEY_COL2__KPP_COL_2 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_KEY_COL3__KPP_COL_3 | MUX_PAD_CTRL(NO_PAD_CTRL)),

	(MX6_PAD_KEY_ROW0__KPP_ROW_0 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
	(MX6_PAD_KEY_ROW1__KPP_ROW_1 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
	(MX6_PAD_KEY_ROW2__KPP_ROW_2 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
	(MX6_PAD_KEY_ROW3__KPP_ROW_3 | MUX_PAD_CTRL(MX6SL_KEYPAD_CTRL)),
};
int setup_mxc_kpd(void)
{
	imx_iomux_v3_setup_multiple_pads(mxc_kpd_pads,
			ARRAY_SIZE(mxc_kpd_pads));

	return 0;
}
#endif /*CONFIG_MXC_KPD*/

#ifdef CONFIG_IMX_UDC
iomux_v3_cfg_t const otg_udc_pads[] = {
	(MX6_PAD_EPDC_PWRCOM__ANATOP_USBOTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL)),
};
void udc_pins_setting(void)
{
	imx_iomux_v3_setup_multiple_pads(otg_udc_pads,
			ARRAY_SIZE(otg_udc_pads));
}
#endif /*CONFIG_IMX_UDC*/


#ifdef CONFIG_CMD_IDME

static const struct board_type *get_board_type(void)
{
    int i;
    if (!board_info_valid((u8 *)gd->board_id, BOARD_PCBA_LEN)) {
	printf("board inf invalid: %s\n", gd->board_id);
	return NULL;
    }

    for (i = 0; i < NUM_KNOWN_BOARDS; i++) {
	if (strncmp((const char *) (gd->board_id + TATOO_CODE_START_OFFSET), boards[i].id, strlen(boards[i].id)) == 0) {
	    return &(boards[i]);
	}
    }

    return NULL;
}

#endif





int checkboard(void)
{

	const char *sn, *rev;
#ifdef CONFIG_CMD_IDME
	const struct board_type *board;
	g_cur_mmc = 0;
	mmc_initialize(gd->bd);
#endif
	
puts("Board: ");
#if CONFIG_CMD_IDME
	board = get_board_type();
	if (board != NULL) {
	    printf("%s\n", board->name);
	} else {
	    printf("Unknown\n");
	}
#else
       puts("Board: Heisenberg\n");
#endif


#ifdef CONFIG_CMD_IDME
        /* serial number and board id */
        sn = (char *) get_board_serial();
        rev = (char *) get_board_id16();

        if (rev)
                printf ("Board Id: %.*s\n", BOARD_PCBA_LEN, rev);

        if (sn)
                printf ("S/N: %.*s\n", BOARD_DSN_LEN, sn);
#endif

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
iomux_v3_cfg_t const usb_otg1_pads[] = {
	MX6_PAD_KEY_COL4__USB_USBOTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EPDC_PWRCOM__ANATOP_USBOTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL)
};

iomux_v3_cfg_t const usb_otg2_pads[] = {
	MX6_PAD_KEY_COL5__USB_USBOTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(usb_otg1_pads,
			ARRAY_SIZE(usb_otg1_pads));
		break;
	case 1:
		imx_iomux_v3_setup_multiple_pads(usb_otg2_pads,
			ARRAY_SIZE(usb_otg2_pads));
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return 1;
	}
	return 0;
}
#endif

#ifdef CONFIG_FSL_MX6SL_LPM_TEST
void board_entry_low_power_mode(void)
{
	disable_pm_epd();
	setup_iomux_lpm();
	gpio_direction_output( IMX_GPIO_NR(5,10), 0);
	gpio_direction_output( IMX_GPIO_NR(5,7), 0);
//as EE required, don't turn off the wifi on GPIO in the lpm, do it with wifi on /off outside of the lpm command.
#if 0
	gpio_direction_output( IMX_GPIO_NR(2,15), 0);
	gpio_direction_output( IMX_GPIO_NR(2,19), 0);
#endif
	gpio_direction_input( IMX_GPIO_NR(3,24));
	gpio_direction_input( IMX_GPIO_NR(4,0));
/*bt uart3*/
	gpio_direction_input( IMX_GPIO_NR(2,3));
	gpio_direction_input( IMX_GPIO_NR(2,4));
	gpio_direction_input( IMX_GPIO_NR(2,5));
	gpio_direction_input( IMX_GPIO_NR(2,6));
/*wifi sdio*/
	gpio_direction_input( IMX_GPIO_NR(5,6));
	gpio_direction_input( IMX_GPIO_NR(5,8));
	gpio_direction_input( IMX_GPIO_NR(5,11));
	gpio_direction_input( IMX_GPIO_NR(5,13));
	gpio_direction_input( IMX_GPIO_NR(5,14));
	gpio_direction_input( IMX_GPIO_NR(5,15));
	
}
#endif

#ifdef CONFIG_BMP_SPLASH_BUILTIN
static void setup_bmp_image(void *image, ulong ilen)
{
	ulong len = ilen;

	gunzip((void *)((uintptr_t)CONFIG_SCRATCH_BUF_ADDR),
	       CONFIG_SCRATCH_BUF_SIZE,
	       image, &len);
	lcd_display_bitmap(CONFIG_SCRATCH_BUF_ADDR, 0, 0);
	/* flush the fb memory. Otherwise, it can look funny */
	flush_cache(gd->fb_base, panel_info.vl_col * panel_info.vl_row);
}

void setup_recovery_bmp(void)
{
	setup_bmp_image(recovery_mode_bmp_gz, recovery_mode_bmp_gz_len);
}

void setup_eol_bmp(void)
{
	setup_bmp_image(splash_bmp_gz, splash_bmp_gz_len);
}

void setup_critbatt_bmp(void)
{
	setup_bmp_image(critbatt_bmp_gz, critbatt_bmp_gz_len);
}

#endif /* CONFIG_BMP_SPLASH_BUILTIN */
