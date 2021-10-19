/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/imx-common/regs-common.h>
#include "../arch-imx/cpu.h"

#define is_soc_rev(rev)		((int)((get_cpu_rev() & 0xFF) - rev))
u32 get_cpu_rev(void);

/* returns MXC_CPU_ value */
#define cpu_type(rev) (((rev) >> 12)&0xff)

/* use with MXC_CPU_ constants */
#define is_cpu_type(cpu) (cpu_type(get_cpu_rev()) == cpu)

const char *get_imx_type(u32 imxtype);
unsigned imx_ddr_size(void);
void set_wdog_reset(struct wdog_regs *wdog);

#ifdef CONFIG_LDO_BYPASS_CHECK
int check_ldo_bypass(void);
int check_1_2G(void);
int set_anatop_bypass(int wdog_reset_pin);
void ldo_mode_set(int ldo_bypass);
void prep_anatop_bypass(void);
void finish_anatop_bypass(void);
#endif

#ifdef CONFIG_MX6SX
int arch_auxiliary_core_up(u32 core_id, u32 boot_private_data);
int arch_auxiliary_core_check_up(u32 core_id);
#endif

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */

int fecmxc_initialize(bd_t *bis);
u32 get_ahb_clk(void);
u32 get_periph_clk(void);

int mxs_reset_block(struct mxs_register_32 *reg);
int mxs_wait_mask_set(struct mxs_register_32 *reg,
		       uint32_t mask,
		       unsigned int timeout);
int mxs_wait_mask_clr(struct mxs_register_32 *reg,
		       uint32_t mask,
		       unsigned int timeout);

#if 1
#define ANDROID_BOOT_MASK		(0xF)
#define ANDROID_BOOT_UNKNOWN		(0)
#define ANDROID_BOOT_FASTBOOT		(5)
#define ANDROID_BOOT_RECOVERY		(6)
#define ANDROID_BOOT_FACTORYRESET	(7)
#define ANDROID_BOOT_KILLPILL		(0xE)
#define ANDROID_BOOT_REGULAR_REBOOT	(0xD)

/* get android boot mode */
u32 imx_get_aboot_mode(void);
void imx_set_aboot_mode(u32 mode);
#endif

#endif
