/*
 * cmd_idme.c 
 *
 * Copyright 2010-12 Amazon Technologies, Inc. All Rights Reserved.
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
#include <mmc.h>
#include <lab126/idme.h>

#define MMC_BLOCK_SIZE	512
DECLARE_GLOBAL_DATA_PTR;

extern int setup_board_info(void);

static const struct nvram_t *idme_find_var(const char *name) 
{
    int i, len;

    for (i = 0; i < CONFIG_NUM_NV_VARS; i++) {

	len = strlen(nvram_info[i].name);
	if (strncmp(name, nvram_info[i].name, len) == 0) {
	    return &nvram_info[i];
	}
    } 

    return NULL;
}

static int idme_print_vars(void) 
{
    int i;
    char value[MAX_IDME_VALUE_SIZE];

    for (i = 0; i < CONFIG_NUM_NV_VARS; i++) {

	idme_get_var(nvram_info[i].name, value, sizeof(value));
	value[nvram_info[i].size] = 0;

	printf("%s: %s\n", nvram_info[i].name, value);
    }

    return 0;
}

/*
 * Read length bytes of idme field
 * Returns:  Zero       success
 *           Non-Zero   error
 */
int idme_get_var_external(const char *name, char *buf, unsigned int length)
{
	return idme_get_var_ex(name, buf, length, false);	
}

/*
 * Update idme field with value that has length bytes
 * Returns:  Zero       success
 *           Non-Zero   error
 */
int idme_update_var_ex(const char *name, const char *value, unsigned int length)
{
	if(idme_update_var_with_size(name, value, length) > 0)
		return 0;
	return -1;
}

/*
 * Fully read idme field. The provided buffer need to big enough to receive value.
 * Returns:  Zero       success
 *           Non-Zero   error
 */
int idme_get_var(const char *name, char *buf, int buflen)
{
	return idme_get_var_ex(name, buf, buflen, true);
}

int idme_get_var_ex(const char *name, char *buf, int buflen, bool read_all)
{
	int ret;
	int address, block, offset, size;
#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	struct mmc *mmc;
#endif
	const struct nvram_t *nv;
	buf[0] = 0;

	nv = idme_find_var(name);
	if (!nv) {
		printf("Error! Couldn't find NV variable %s\n", name);
		return -1;
	}

	block = nv->offset / MMC_BLOCK_SIZE;
	address = CONFIG_MMC_USERDATA_ADDR + (block * MMC_BLOCK_SIZE);
	offset = nv->offset % MMC_BLOCK_SIZE;
	size = nv->size;
#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)

	mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
	if (mmc == NULL) {
		printf("Error: Couldn't find flash device");
		return -1;
	}

	mmc_init(mmc);
	if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, CONFIG_BOOT_FROM_PARTITION) < 0) {
	printf("%s ERROR: couldn't switch to boot partition\n", __FUNCTION__);
	return -1;
	}
#endif
	mmc->part_num = CONFIG_BOOT_FROM_PARTITION;
	//ret = mmc_read(CONFIG_MMC_BOOTFLASH, address, gd->varbuf, MMC_BLOCK_SIZE);
	ret = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, address/MMC_BLOCK_SIZE,1, (void *) gd->varbuf);
	flush_cache((ulong)gd->varbuf, 512); /* FIXME */
	if (0 == ret) {
		printf("%s error! Couldn't read vars from partition\n", __FUNCTION__);
		/* need to switch back to user partition even on error */
	}


#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) {
	printf("%s ERROR: couldn't switch back to user partition\n", __FUNCTION__);
	return -1;
	}
	mmc->part_num = 0;
#endif

	if (0 == ret) {
		return -1;
	}

	if (read_all) {
		// buffer is too small, return needed size
		if (size + 1 > buflen) {
			return (size + 1);
		}
	}
	else {
		size = MIN(size, buflen);
	}

	memcpy(buf, (void *)gd->varbuf + offset, size);
	buf[size] = 0;
	return 0;
}

/*
 * Update idme field with value that has val_size bytes.
 * If value is null-terminated string, passing val_size=-1.
 *
 * Returns:  > 0    Number of block read
 *           <=0	Error
 */
int idme_update_var_with_size(const char *name, const char *value, int val_size)
{
	int ret = 0, sz;
	int address, block, offset, size;
	#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	struct mmc *mmc;
	#endif
	const struct nvram_t *nv;

	nv = idme_find_var(name);
	if (!nv) {
		printf("Error! Couldn't find NV variable %s\n", name);
		return -1;
	}

	block = nv->offset / MMC_BLOCK_SIZE;
	address = CONFIG_MMC_USERDATA_ADDR + (block * MMC_BLOCK_SIZE);
	offset = nv->offset % MMC_BLOCK_SIZE;
	size = nv->size;

	#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)

	mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
	if (mmc == NULL) {
		printf("Error: Couldn't find flash device");
		return -1;
	}

	mmc_init(mmc);
	if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, CONFIG_BOOT_FROM_PARTITION) < 0) {
		printf("%s ERROR: couldn't switch to boot partition\n", __FUNCTION__);
		return -1;
	}
	mmc->part_num = CONFIG_BOOT_FROM_PARTITION;
	#endif

	//ret = mmc_read(CONFIG_MMC_BOOTFLASH, address, gd->varbuf, MMC_BLOCK_SIZE);
	ret = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, address/MMC_BLOCK_SIZE,1, (void *) gd->varbuf);
	flush_cache((ulong)gd->varbuf, 512); /* FIXME */


	if (0 == ret) {
		printf("Error! Couldn't read NV variables. (%d)\n", ret);
		goto out;
	}

	// clear out old value
	memset((void *)gd->varbuf + offset, 0, size);

	if (value != NULL) {
		// copy in new value
		sz = val_size!=-1 ? val_size : MIN(size, strlen(value));
		memcpy((void *)gd->varbuf + offset, value, sz);
	}

	//ret = mmc_write(CONFIG_MMC_BOOTFLASH, gd->varbuf, address, MMC_BLOCK_SIZE);
	ret = mmc->block_dev.block_write(CONFIG_MMC_BOOTFLASH, address/MMC_BLOCK_SIZE,1, (void *) gd->varbuf);
	if (0 == ret) {
		printf("Error! Couldn't write NV variables. (%d)\n", ret);
		/* need to switch back to user partition even on error */
	}

out:
	#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	if ((!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) || (0 == ret)) {
		printf("%s ERROR: couldn't switch back to user partition\n", __FUNCTION__);
		return -1;
	}
	mmc->part_num = 0;
	#endif

	/* Make sure that the board info globals (DSN/pcbsn) are updated as well */
	setup_board_info();

	return ret;
}

/*
 * Read idme field as null-terminated string
 * Returns:  Zero       success
 *           Non-Zero   error
 */
int idme_update_var(const char *name, const char *value) 
{
	return idme_update_var_with_size(name, value, -1);
}

/*
 * Check whether user code has updated idme, and import new values
 * Returns:  1 if an update was imported
 *           0 if there was an error or no update (no point clearing afterward)
 */
int idme_check_update(void) {

    int ret = 0;

#ifdef CONFIG_IDME_UPDATE
    struct mmc *mmc;
    int len = 0;

    mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
    if (mmc == NULL) {
	printf("%s Error: Couldn't find flash device", __FUNCTION__);
	return 0;
    }

    mmc_init(mmc);
    //ret = mmc_read(CONFIG_MMC_BOOTFLASH, CONFIG_IDME_UPDATE_ADDR, gd->varbuf, MMC_BLOCK_SIZE);
    ret = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, CONFIG_IDME_UPDATE_ADDR/MMC_BLOCK_SIZE,1, (void *)gd->varbuf);
    flush_cache((ulong)gd->varbuf, 512); /* FIXME */
    if (0 == ret) {
	printf("Error! Couldn't read NV variables. (%d)\n", ret);
	return 0;
    }
    
    /* Check to see if we need to update idme vars */
    if (strncmp((char *) (gd->varbuf + (MMC_BLOCK_SIZE - CONFIG_IDME_UPDATE_MAGIC_SIZE)), 
		CONFIG_IDME_UPDATE_MAGIC,
		CONFIG_IDME_UPDATE_MAGIC_SIZE) == 0) {
	
	printf("Found updated idme variables.  Flashing...\n");
	
	while (len < CONFIG_MMC_USERDATA_SIZE) {

	    if (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, CONFIG_BOOT_FROM_PARTITION) < 0) {
		printf("%s ERROR: couldn't switch to boot partition\n", __FUNCTION__);
		return 0;
	    }
            mmc->part_num = CONFIG_BOOT_FROM_PARTITION;
	    //ret = mmc_write(CONFIG_MMC_BOOTFLASH, gd->varbuf, CONFIG_MMC_USERDATA_ADDR + len, MMC_BLOCK_SIZE);
            ret = mmc->block_dev.block_write(CONFIG_MMC_BOOTFLASH, (CONFIG_MMC_USERDATA_ADDR + len)/MMC_BLOCK_SIZE,1, (void *) gd->varbuf);
	    if (0 == ret) {
		printf("Error! Couldn't write NV variables. (%d)\n", ret);
		/* need to switch back to user partition even on error */
	    }

	    if ( (!IS_SD(mmc) && mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) || (0 == ret )) {
		printf("%s ERROR: couldn't switch back to user partition\n", __FUNCTION__);
		return 0;
	    }
            mmc->part_num = 0;
	    len += MMC_BLOCK_SIZE;

	    //ret = mmc_read(CONFIG_MMC_BOOTFLASH, CONFIG_IDME_UPDATE_ADDR + len, gd->varbuf, MMC_BLOCK_SIZE);
            ret = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, (CONFIG_IDME_UPDATE_ADDR + len)/MMC_BLOCK_SIZE,1, (void *) gd->varbuf);
            flush_cache((ulong)gd->varbuf, 512); /* FIXME */
            if (0 == ret) {
		printf("Error! Couldn't read NV variables. (%d)\n", ret);
		return 0;
	    }
	}

	printf("idme variable flash complete\n");
	return 1;
    }    
#endif /* CONFIG_IDME_UPDATE */

    return ret;
}

/*
 * Clear out user updates to idme
 * Returns: 0 on success
 *          !=0 on failure
 */
int idme_clear_update(void)
{
#ifdef CONFIG_IDME_UPDATE
	struct mmc *mmc;
	int len = 0, ret = -1;

	mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
	if (mmc == NULL) {
		printf("%s Error: Couldn't find flash device", __FUNCTION__);
		return ret;
	}

        mmc_init(mmc);
	//ret = mmc_read(CONFIG_MMC_BOOTFLASH, CONFIG_IDME_UPDATE_ADDR, gd->varbuf, MMC_BLOCK_SIZE);
        ret = mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, CONFIG_IDME_UPDATE_ADDR/MMC_BLOCK_SIZE,1, (void *) gd->varbuf);
        flush_cache((ulong)gd->varbuf, 512); /* FIXME */
        if (0 == ret) {
		printf("Error! Couldn't read NV variables. (%d)\n", ret);
		return ret;
	}
    
	/*
	 * Only clear the update region if there it contains update data
	 * (If there is no update there, there's no point clearing it).
	 */
	if (strncmp((char *) (gd->varbuf + (MMC_BLOCK_SIZE - CONFIG_IDME_UPDATE_MAGIC_SIZE)), 
			CONFIG_IDME_UPDATE_MAGIC,
			CONFIG_IDME_UPDATE_MAGIC_SIZE) == 0) {
	
		puts("Clearing idme update...");
		memset((void *)gd->varbuf, 0, MMC_BLOCK_SIZE);	
		while (len < CONFIG_MMC_USERDATA_SIZE) {
			//ret = mmc_write(CONFIG_MMC_BOOTFLASH, gd->varbuf, CONFIG_IDME_UPDATE_ADDR + len, MMC_BLOCK_SIZE);
                        ret = mmc->block_dev.block_write(CONFIG_MMC_BOOTFLASH, (CONFIG_IDME_UPDATE_ADDR + len)/MMC_BLOCK_SIZE,1, (unsigned char *) gd->varbuf);
			if (0 == ret) {
				printf("Error! Couldn't clear update variables. (%d)\n", ret);
				return ret;
			}
			len += MMC_BLOCK_SIZE;
		}
		puts("done\n");
	}
#endif /* CONFIG_IDME_UPDATE */
	return 0;
}

static int do_idme (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc == 1 || strncmp(argv[1], "-s", 2) == 0) {
	return idme_print_vars();
    } else if (argc <= 3) {
	char *value; 

	if (argc == 3) {
	    value = argv[2];
	    printf("setting '%s' to '%s'\n", argv[1], argv[2]);
	} else {
	    value = NULL;
	    printf("clearing '%s'\n", argv[1]);
	}

	return idme_update_var(argv[1], value);
    }

    printf ("Usage:\n%s", cmdtp->usage);
    return 1;	
}

/***************************************************/

U_BOOT_CMD(
	idme,	3,	1,	do_idme,
	"idme    - Set nv ram variables",
	"idme <var> <value>\n"
	"    - set a variable value\n"
	"idme -s"
	"    - print out known values\n"
);
