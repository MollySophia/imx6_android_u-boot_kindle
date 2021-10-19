/*
 * cmd_checkcrc.c
 *
 * Copyright 2010 Amazon Technologies, Inc. All Rights Reserved.
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

#include <boardid.h>

#define MMC_BLOCK_SIZE  512
#define PARTITION_FILL_SPACE    -1
typedef struct partition_info_t {
    const char *name;
    unsigned int address;
    unsigned int size;
    unsigned int partition;
} partition_info_t;

int mmc_crc32_test (int part, uint start, int size, uint crc);

extern const struct partition_info_t partition_info_default[CONFIG_NUM_PARTITIONS];
const struct partition_info_t *get_partition_info_for_device(void)
{
	return partition_info_default;
}


int do_check_crc (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int size, i, part = -1;
	uint start, crc;
	const struct partition_info_t *partition_info = get_partition_info_for_device();

	debug("argc=[%d], argv=[0,%s],[1,%s],[2,%s],[3,%s],[4,%s],flag=0x%08x\n",
		argc, argv[0], argv[1], argv[2], argv[3], argv[4], flag);

	if (argc == 5) {
		for (i = 0; i < CONFIG_NUM_PARTITIONS; i++) {

			if (strcmp(argv[1], partition_info[i].name) == 0) {
				part = i;
				break;
			} else debug("%s <> %s\n", argv[1], partition_info[i].name);
		}

		if (part < 0) {
#if defined(CONFIG_MX35)
			printf("image must be bootloader, kernel or system\n");
#else
			printf("image must be %s", partition_info[0].name);
			for (i = 1; i < CONFIG_NUM_PARTITIONS; i++) {
				printf(", %s", partition_info[i].name);
			}
			printf("\n");
#endif
			return -1;
		}

		start = (uint)simple_strtoul(argv[2], NULL, 16);
		size = (uint)simple_strtoul(argv[3], NULL, 16);
		crc = (uint)simple_strtoul(argv[4], NULL, 16);

		debug("crc32 test, part=0x%08x, start=0x%08x, size=0x%08x, crc=0x%08x\n", part, start, size, crc);

		return mmc_crc32_test(part, start, size, crc);
	} else {
		printf("usage: check image start size crc\n");
		return -1;
	}
}
/***************************************************/

U_BOOT_CMD(
	check,	5,	0,	do_check_crc,
	"perform MMC CRC32 check",
	"image start size crc\n"
	"    - images:\n"
	"    - bootloader comes from u-boot.bin\n"
	"    - kernel comes from uImage\n"
	"    - system comes from rootfs.img"
);


struct crc_table_t {
	u32 size;
	u32 crc;
};

int mmc_crc32_read (int part, u32 src, uchar *dst, u32 size)
{
	int err = 0;
#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	struct mmc *mmc;
	const struct partition_info_t *partition_info = get_partition_info_for_device();

	mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
	if (mmc == NULL) {
		printf("ERROR: Couldn't find flash device");
		return -1;
	}

	if (mmc_switch_part(CONFIG_MMC_BOOTFLASH, partition_info[part].partition) < 0) {
		printf("ERROR: couldn't switch to partition\n");
		return -1;
	}
	mmc->part_num = CONFIG_BOOT_FROM_PARTITION;

	if (size % mmc->read_bl_len)
		size = size + (mmc->read_bl_len - (size % mmc->read_bl_len));
#endif

	debug("read src(mmc)=0x%08x, dst=0x%08x, size=0x%08x\n", src, (u32)dst, size);
	//mmc_read(CONFIG_MMC_BOOTFLASH, src, dst, size);
	mmc->block_dev.block_read(CONFIG_MMC_BOOTFLASH, src/MMC_BLOCK_SIZE,size/MMC_BLOCK_SIZE, dst);

#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	if (mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) {
		printf("ERROR: couldn't switch back to user partition\n");
		return -1;
	}
	mmc->part_num = 0;
#endif
	return err;
}

int mmc_crc32_write (int part, uchar *src, u32 dst, u32 size)
{
	int err = 0;
#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	struct mmc *mmc;
	const struct partition_info_t *partition_info = get_partition_info_for_device();


	mmc = find_mmc_device(CONFIG_MMC_BOOTFLASH);
	if (mmc == NULL) {
		printf("ERROR: Couldn't find flash device");
		return -1;
	}

	if (mmc_switch_part(CONFIG_MMC_BOOTFLASH, partition_info[part].partition) < 0) {
		printf("ERROR: couldn't switch to partition\n");
		return -1;
	}
	mmc->part_num = CONFIG_BOOT_FROM_PARTITION;
#endif

	debug("write src=0x%08x, dst(mmc)=0x%08x, size=0x%08x\n", (u32)src, dst, size);
	//mmc_write(CONFIG_MMC_BOOTFLASH, src, dst, size);
	mmc->block_dev.block_write(CONFIG_MMC_BOOTFLASH, dst/MMC_BLOCK_SIZE,size/MMC_BLOCK_SIZE, src);
#if defined(CONFIG_BOOT_PARTITION_ACCESS) && defined(CONFIG_BOOT_FROM_PARTITION)
	if (mmc_switch_part(CONFIG_MMC_BOOTFLASH, 0) < 0) {
		printf("ERROR: couldn't switch back to user partition\n");
		return -1;
	}
	mmc->part_num = CONFIG_BOOT_FROM_PARTITION;
#endif
	return err;
}

int mmc_crc32_test (int part, uint start, int size, uint crc)
{
	uint checkaddr;
	int i, runsize, checksize;
	struct crc_table_t crc_table;
	uint32_t crc_cac = ~0U; /* initial */
	const struct partition_info_t *partition_info = get_partition_info_for_device();


	checkaddr = partition_info[part].address;
	checksize = partition_info[part].size;

	/* Don't check unbounded partitions */
	if (checksize == PARTITION_FILL_SPACE) {
		return 0;
	}

	debug("check %s : ", partition_info[part].name);

	debug("checkaddr=0x%08x, checksize=0x%08x\n", checkaddr, checksize);

	if (start == 1 && size == 1 && crc == 1) {
		mmc_crc32_read(part, checkaddr + checksize - 512, (unsigned char*)CRC32_BUFFER, 512); //mmc->read_bl_len
		memcpy(&crc_table, (unsigned char*)CRC32_BUFFER + 512 - (2 * sizeof(u32)), 2 * sizeof(u32));
		size = crc_table.size;
		crc = crc_table.crc;
		start = checkaddr;
	}

	debug("part=0x%08x, start=0x%08x, size=0x%08x, crc=0x%08x\n", part, start, size, crc);

	if (start != checkaddr || (unsigned)size > checksize) {
		printf("%s ERROR: section parameters incorrect\n", partition_info[part].name);
		return -1;
	}

	runsize = size;
	if (size > CRC32_BUFFER_SIZE)
		runsize = CRC32_BUFFER_SIZE;

	debug("start=0x%08x, runsize=0x%08x\n", start, runsize);
	mmc_crc32_read(part, start, (unsigned char*)CRC32_BUFFER, runsize);

  	crc_cac=crc32_no_comp(crc_cac, (unsigned char*)CRC32_BUFFER, runsize);
	debug("runsize=0x%08x, crc(cac)=0x%08x\n", runsize, crc_cac);
	printf(".");

  	for (i = CRC32_BUFFER_SIZE; i < (size - CRC32_BUFFER_SIZE); i += CRC32_BUFFER_SIZE) {
		mmc_crc32_read(part, start + i, (unsigned char*)CRC32_BUFFER, CRC32_BUFFER_SIZE);
	  	crc_cac=crc32_no_comp(crc_cac,(unsigned char*)CRC32_BUFFER, CRC32_BUFFER_SIZE);
		debug("runleft=0x%08x, crc(cac)=0x%08x\n", size - i, crc_cac);
		printf(".");
	}

	runsize  = size - i;
	if (runsize  > 0) {
		mmc_crc32_read(part, start + i, (unsigned char*)CRC32_BUFFER, runsize);
  		crc_cac=crc32_no_comp(crc_cac, (unsigned char*)CRC32_BUFFER, runsize);
		debug("runsize=0x%08x, crc(cac)=0x%08x\n", runsize, crc_cac);
		printf(".");
	}

	crc_cac=~crc_cac; /* we invert it at the end */
	debug("size=0x%08x, crc(cac)=0x%08x\n", size, crc_cac);

	if (crc == crc_cac) {
		printf("%s PASS\n", partition_info[part].name);
		debug("start=0x%08x, size=0x%08x, crc(mmc)=0x%08x\n", start, size, crc);

		crc_table.crc = crc;
		crc_table.size = size;
		memset((unsigned char*)CRC32_BUFFER, 0xFF, 512);
		memcpy((unsigned char*)CRC32_BUFFER + 512 - (2 * sizeof(u32)), &crc_table, 2 * sizeof(u32));
		mmc_crc32_write(part, (unsigned char*)CRC32_BUFFER, checkaddr + checksize - 512, 512); //mmc->read_bl_len
		return 0;
	} else {
		printf("%s FAIL\n", partition_info[part].name);
		debug("start=0x%08x, checksize=0x%08x, crc(mmc)=0x%08x, crc(cac)=0x%08x\n",
			  start, size, crc, crc_cac);
		return -1;
	}
}

static int mmc_crc32_find_partition(const char *partition_name)
{
	int i, len;
	const struct partition_info_t *partition_info = get_partition_info_for_device();

	for (i = 0; i < CONFIG_NUM_PARTITIONS; i++) {
		len = strlen(partition_info[i].name);
		if (strncmp(partition_name, partition_info[i].name, len) == 0) {
			return i;
		}
	}

	return -1;
}

/*
Example on how to set flag to CONFIG_SYS_POST_MMC_CRC32 in file "test.c".
#if CONFIG_POST & CONFIG_SYS_POST_MMC_CRC32
    {
	"MMC CRC32 test",
	"mmc_crc32",
	"This tests the CRC32 of the MMC.",
	POST_RAM | POST_MANUAL,	// to turn on crc check for u-boot.bin ==> change to CRC32_CHECK_UBOOT| POST_RAM | POST_MANUAL,
	&mmc_crc32_post_test,
	NULL,
	NULL,
	CONFIG_SYS_POST_MMC_CRC32
    },
#endif
*/

int mmc_crc32_post_test (int flags)
{
	int i;
	int part = -1;
	int result0 = 0;

	if (flags & CRC32_CHECK_UBOOT)
		part = mmc_crc32_find_partition("uboot");
	else if (flags & CRC32_CHECK_MBR)
		part = mmc_crc32_find_partition("mbr");
	else if (flags & CRC32_CHECK_UIMAGE)
		part = mmc_crc32_find_partition("kernel");
	else if (flags & CRC32_CHECK_ROOTFS)
		part = mmc_crc32_find_partition("system");

	if (part < 0) {
	  	for (i = 0; i < CONFIG_NUM_PARTITIONS; i++) {
			debug("crc32 test, part=0x%08x\n", i);
			result0 |= mmc_crc32_test(i, 1, 1, 1);
		}
	} else {
		debug("crc32 test, flags=0x%08x, part=0x%08x\n", flags, part);
		result0 = mmc_crc32_test(part, 1, 1, 1);
	}

	return (result0);
}


