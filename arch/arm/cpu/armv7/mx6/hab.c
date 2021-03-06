/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hab.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

/* -------- start of HAB API updates ------------*/
#define hab_rvt_report_event_p						\
(									\
	((is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D)) \
		&& (is_soc_rev(CHIP_REV_1_5) >= 0)) ?		\
	((hab_rvt_report_event_t *)HAB_RVT_REPORT_EVENT_NEW) :		\
	(is_cpu_type(MXC_CPU_MX6DL) && (is_soc_rev(CHIP_REV_1_2) >= 0)) ?		\
	((hab_rvt_report_event_t *)HAB_RVT_REPORT_EVENT_NEW) :		\
	((hab_rvt_report_event_t *)HAB_RVT_REPORT_EVENT)		\
)

#define hab_rvt_report_status_p						\
(									\
	((is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D)) \
		&& (is_soc_rev(CHIP_REV_1_5) >= 0)) ?		\
	((hab_rvt_report_status_t *)HAB_RVT_REPORT_STATUS_NEW) :	\
	((is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))	\
		&& (is_soc_rev(CHIP_REV_1_2) >= 0)) ?		\
	((hab_rvt_report_status_t *)HAB_RVT_REPORT_STATUS_NEW) :	\
	((hab_rvt_report_status_t *)HAB_RVT_REPORT_STATUS)		\
)

#define hab_rvt_authenticate_image_p					\
(									\
	((is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))	\
		&& (is_soc_rev(CHIP_REV_1_5) >= 0)) ?		\
	((hab_rvt_authenticate_image_t *)HAB_RVT_AUTHENTICATE_IMAGE_NEW) : \
	((is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))	\
		&& (is_soc_rev(CHIP_REV_1_2) >= 0)) ?		\
	((hab_rvt_authenticate_image_t *)HAB_RVT_AUTHENTICATE_IMAGE_NEW) :\
	((hab_rvt_authenticate_image_t *)HAB_RVT_AUTHENTICATE_IMAGE)	\
)

#define hab_rvt_entry_p							\
(									\
	((is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))	\
		&& (is_soc_rev(CHIP_REV_1_5) >= 0)) ?		\
	((hab_rvt_entry_t *)HAB_RVT_ENTRY_NEW) :			\
	((is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))	\
		&& (is_soc_rev(CHIP_REV_1_2) >= 0)) ?		\
	((hab_rvt_entry_t *)HAB_RVT_ENTRY_NEW) :			\
	((hab_rvt_entry_t *)HAB_RVT_ENTRY)				\
)

#define hab_rvt_exit_p							\
(									\
	((is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))	\
		&& (is_soc_rev(CHIP_REV_1_5) >= 0)) ?		\
	((hab_rvt_exit_t *)HAB_RVT_EXIT_NEW) :				\
	((is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))	\
		&& (is_soc_rev(CHIP_REV_1_2) >= 0)) ?		\
	((hab_rvt_exit_t *)HAB_RVT_EXIT_NEW) :				\
	((hab_rvt_exit_t *)HAB_RVT_EXIT)				\
)

#define IVT_SIZE		0x20
#define ALIGN_SIZE		0x1000
#define CSF_PAD_SIZE		0x2000

/*
 * +------------+  0x0 (DDR_UIMAGE_START) -
 * |   Header   |                          |
 * +------------+  0x40                    |
 * |            |                          |
 * |            |                          |
 * |            |                          |
 * |            |                          |
 * | Image Data |                          |
 * .            |                          |
 * .            |                           > Stuff to be authenticated ----+
 * .            |                          |                                |
 * |            |                          |                                |
 * |            |                          |                                |
 * +------------+                          |                                |
 * |            |                          |                                |
 * | Fill Data  |                          |                                |
 * |            |                          |                                |
 * +------------+ Align to ALIGN_SIZE      |                                |
 * |    IVT     |                          |                                |
 * +------------+ + IVT_SIZE              -                                 |
 * |            |                                                           |
 * |  CSF DATA  | <---------------------------------------------------------+
 * |            |
 * +------------+
 * |            |
 * | Fill Data  |
 * |            |
 * +------------+ + CSF_PAD_SIZE
 */

bool is_hab_enabled(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;
	uint32_t reg = readl(&fuse->cfg5);

	return (reg & 0x2) == 0x2;
}

void display_event(uint8_t *event_data, size_t bytes)
{
	uint32_t i;

	if (!(event_data && bytes > 0))
		return;

	for (i = 0; i < bytes; i++) {
		if (i == 0)
			printf("\t0x%02x", event_data[i]);
		else if ((i % 8) == 0)
			printf("\n\t0x%02x", event_data[i]);
		else
			printf(" 0x%02x", event_data[i]);
	}
}

int get_hab_status(void)
{
	uint32_t index = 0; /* Loop index */
	uint8_t event_data[128]; /* Event data buffer */
	size_t bytes = sizeof(event_data); /* Event size in bytes */
	enum hab_config config = 0;
	enum hab_state state = 0;
	hab_rvt_report_event_t *hab_rvt_report_event;
	hab_rvt_report_status_t *hab_rvt_report_status;

	if (is_hab_enabled())
		puts("\nSecure boot enabled\n");
	else
		puts("\nSecure boot disabled\n");

	hab_rvt_report_event = hab_rvt_report_event_p;
	hab_rvt_report_status = hab_rvt_report_status_p;

	/* Check HAB status */
	if (hab_rvt_report_status(&config, &state) != HAB_SUCCESS) {
		printf("\nHAB Configuration: 0x%02x, HAB State: 0x%02x\n",
		       config, state);

		/* Display HAB Error events */
		while (hab_rvt_report_event(HAB_FAILURE, index, event_data,
					&bytes) == HAB_SUCCESS) {
			puts("\n");
			printf("--------- HAB Event %d -----------------\n",
			       index + 1);
			puts("event data:\n");
			display_event(event_data, bytes);
			puts("\n");
			bytes = sizeof(event_data);
			index++;
		}
		printf("HAB Error: %d\n", -index);

		return -index;
	}
	/* Display message if no HAB events are found */
	else {
		printf("\nHAB Configuration: 0x%02x, HAB State: 0x%02x\n",
		       config, state);
		puts("No HAB Events Found!\n\n");
	}
	return 0;
}

#ifdef DEBUG_AUTHENTICATE_IMAGE
void dump_mem(uint32_t addr, int size)
{
	int i;

	for (i = 0; i < size; i += 4) {
		if (i != 0) {
			if (i % 16 == 0)
				printf("\n");
			else
				printf(" ");
		}

		printf("0x%08x", *(uint32_t *)addr);
		addr += 4;
	}

	printf("\n");

	return;
}
#endif

uint32_t authenticate_image(uint32_t ddr_start, uint32_t image_size)
{
	uint32_t load_addr = 0;
	size_t bytes;
	ptrdiff_t ivt_offset = 0;
	int result = 0;
	ulong start;
	hab_rvt_authenticate_image_t *hab_rvt_authenticate_image;
	hab_rvt_entry_t *hab_rvt_entry;
	hab_rvt_exit_t *hab_rvt_exit;

	hab_rvt_authenticate_image = hab_rvt_authenticate_image_p;
	hab_rvt_entry = hab_rvt_entry_p;
	hab_rvt_exit = hab_rvt_exit_p;

	// Verify image even SEC_CONFIG is opened
	{
		printf("\nAuthenticate image from DDR location 0x%x...\n",
			ddr_start);

		hab_caam_clock_enable();

		if (hab_rvt_entry() == HAB_SUCCESS) {
			// already aligned
			ivt_offset = image_size - CSF_PAD_SIZE - IVT_SIZE;

			start = ddr_start;
			bytes = image_size;

#ifdef DEBUG_AUTHENTICATE_IMAGE
			printf("\nivt_offset = 0x%x, ivt addr = 0x%x\n",
			       ivt_offset, ddr_start + ivt_offset);
			printf("Dumping IVT\n");
			dump_mem(ddr_start + ivt_offset, 0x20);

			printf("Dumping CSF Header\n");
			dump_mem(ddr_start + ivt_offset + 0x20, 0x40);

			get_hab_status();

			printf("\nCalling authenticate_image in ROM\n");
			printf("\tivt_offset = 0x%x\n\tstart = 0x%08x"
			       "\n\tbytes = 0x%x\n", ivt_offset, start, bytes);
#endif
			/*
			 * If the MMU is enabled, we have to notify the ROM
			 * code, or it won't flush the caches when needed.
			 * This is done, by setting the "pu_irom_mmu_enabled"
			 * word to 1. You can find its address by looking in
			 * the ROM map. This is critical for
			 * authenticate_image(). If MMU is enabled, without
			 * setting this but, authentication will fail and may
			 * crash.
			 */
			if (is_cpu_type(MXC_CPU_MX6Q) ||
			    is_cpu_type(MXC_CPU_MX6D)) {
				/*
				 * This won't work on Rev 1.0.0 of i.MX6Q/D,
				 * since their ROM doesn't do cache flushes.
				 * I don't think any exist, so we ignore them.
				 */
				writel(1, 0x009024a8);
			} else if (is_cpu_type(MXC_CPU_MX6DL) ||
				   is_cpu_type(MXC_CPU_MX6SOLO)) {
				writel(1, 0x00901dd0);
			} else if (is_cpu_type(MXC_CPU_MX6SL)) {
				writel(1, 0x00900a18);
			}

			load_addr = (uint32_t)hab_rvt_authenticate_image(
					HAB_CID_UBOOT,
					ivt_offset, (void **)&start,
					(size_t *)&bytes, NULL);
			if (hab_rvt_exit() != HAB_SUCCESS) {
				printf("HAB exit function fail\n");
				load_addr = 0;
			}
		} else
			printf("HAB entry function fail\n");

		hab_caam_clock_disable();

		if (load_addr != 0 && get_hab_status() == 0) {
			printf("HAB success!\n");
			result = 1;
		}
		else {
			printf("HAB fail!\n");
			result = 1;
		}
	}

	return result;
}

int do_hab_status(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if ((argc != 1)) {
		cmd_usage(cmdtp);
		return 1;
	}

	get_hab_status();

	return 0;
}

static int do_authenticate_image(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	ulong	addr, ivt_offset;
	int	rcode = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], NULL, 16);
	ivt_offset = simple_strtoul(argv[2], NULL, 16);

	rcode = authenticate_image(addr, ivt_offset);

	return rcode;
}

U_BOOT_CMD(
		hab_status, CONFIG_SYS_MAXARGS, 1, do_hab_status,
		"display HAB status",
		""
	  );

U_BOOT_CMD(
		hab_auth_img, 3, 1, do_authenticate_image,
		"authenticate image via HAB",
		"addr ivt_offset\n"
		"addr - image hex address\n"
		"ivt_offset - hex offset of IVT in the image"
	  );
