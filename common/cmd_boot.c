/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <mmc.h>

#ifdef CONFIG_CMD_GO

/* Allow ports to override the default behavior */
__attribute__((weak))
unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
				 char * const argv[])
{
	return entry (argc, argv);
}

static int do_go(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong	addr, rc;
	int     rcode = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting application at 0x%08lX ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_go_exec ((void *)addr, argc - 1, argv + 1);
	if (rc != 0) rcode = 1;

	printf ("## Application terminated, rc = 0x%lX\n", rc);
	return rcode;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	go, CONFIG_SYS_MAXARGS, 1,	do_go,
	"start application at address 'addr'",
	"addr [arg ...]\n    - start application at address 'addr'\n"
	"      passing 'arg' as arguments"
);

#endif

U_BOOT_CMD(
	reset, 1, 0,	do_reset,
	"Perform RESET of the CPU",
	""
);


#if defined(CONFIG_CMD_MBOOT) && defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_MMC_MBOOT_PARTITION)
int load_mboot()
{
    int blk;
    int i, ret;
    struct mmc *mmc;

    mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
    if (mmc == NULL) {
        printf("Error: Couldn't find flash device");
        return -1;
    }
    mmc_init(mmc);

    if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, CONFIG_MMC_MBOOT_PARTITION) < 0) {
        printf("%s ERROR: couldn't switch to boot partition\n", __FUNCTION__);
        return -1;
    }

    blk = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, CONFIG_MMC_MBOOT_ADDR/512,
        CONFIG_MMC_MBOOT_SIGNED_SIZE/512, (unsigned char *) CONFIG_MBOOTADDR);

    if (blk == 0) {
        printf("ERROR: couldn't read main boot image from flash address 0x%x\n", \
               CONFIG_MMC_MBOOT_ADDR);
        return -1;
    }

    if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) {
        printf("%s ERROR: couldn't switch to user partition\n", __FUNCTION__);
        return -1;
    }

    flush_cache((unsigned long) CONFIG_MBOOTADDR, CONFIG_MMC_MBOOT_SIGNED_SIZE);

    // Verify image
    {
        extern bool is_hab_enabled(void);
        extern uint32_t authenticate_image(uint32_t ddr_start, uint32_t image_size);
        int authen_result = authenticate_image(CONFIG_MBOOTADDR, CONFIG_MMC_MBOOT_SIGNED_SIZE);
        if (authen_result) {
            printf("Passed HAB authentication!\n");
        }
        else {
            if (is_hab_enabled()) {
                printf("Failed HAB authentication!\n");
                return -1;
            }
            else {
                printf("SEC_CONFIG is opened. Bypass HAB authentication!\n");
            }
        }
    }

    // Jump to main u-boot
    ulong (*mboot_func)(int, char * const []) = CONFIG_MBOOTADDR;
    mboot_func(0, "");

    return 0;
}

int do_mboot (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    return load_mboot();
}

U_BOOT_CMD(
        mboot, CONFIG_SYS_MAXARGS, 0,      do_mboot,
        "start main bootloader",
        ""
);
#endif


#ifdef CONFIG_CMD_BIST
int do_bist (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
        int err;
        int i, ret;
        char *cmd = (char *) CONFIG_BISTCMD_LOCATION;
        struct mmc *mmc;

        mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
        if (mmc == NULL) {
            printf("Error: Couldn't find flash device");
            return -1;
        }
	mmc_init(mmc);

#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
       if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, CONFIG_BOOT_FROM_PARTITION) < 0) {
        printf("%s ERROR: couldn't switch to boot partition\n", __FUNCTION__);
        return -1;
    	}
#endif

	err = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, CONFIG_MMC_BIST_ADDR/512,CONFIG_MMC_BIST_SIZE/512, (unsigned char *) CONFIG_BISTADDR);
       if (err == 0) {
                printf("ERROR: couldn't read bist image from flash address 0x%x\n", \
                           CONFIG_MMC_BIST_ADDR);
                return err;
        }

#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
      if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) {
        printf("%s ERROR: couldn't switch to user partition\n", __FUNCTION__);
        return -1;
        }
#endif
 
        cmd[0] = CONFIG_BISTCMD_MAGIC;
        cmd[1] = 0;

        /* copy cmd arguments to saved mem location */
        cmd = &(cmd[1]);
        for (i = 1; i < argc; i++) {
            ret = sprintf(cmd, "%s ", argv[i]);
            if (ret > 0) {
                cmd += ret;
            } else {
                break;
            }
        }

        /* null terminate */
        cmd[0] = 0;
        flush_cache((unsigned long) CONFIG_BISTADDR,CONFIG_MMC_BIST_SIZE);
        err = do_go_exec ((void *)(CONFIG_BISTADDR), argc - 1, argv + 1);
        printf ("## Application terminated, rc = 0x%X\n", err);
	
	return 0;
}

U_BOOT_CMD(
        bist, CONFIG_SYS_MAXARGS, 0,      do_bist,
        "start Built In Self Test",
        ""
);
#endif

