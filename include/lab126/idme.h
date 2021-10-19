/*
 * idme.h 
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

/*!
 * @file idme.h
 * @brief This file contains functions for interacting with variables
 *	in the userstore partition
 *
 */

#ifndef __IDME_H__
#define __IDME_H__

/*
 * Do not modify these length fields unless you know what you are doing.
 * Changing these can easily cause data corruption 
 */
#define BOARD_DSN_LEN   16
#define BOARD_FSN_LEN   13
#define BOARD_MAC_LEN 12
#define BOARD_SEC_LEN 20
#define BOARD_PCBA_LEN  16
#define BOARD_BOOTMODE_LEN 16
#define BOARD_POSTMODE_LEN 16
#define BOARD_QBCOUNT_LEN 10
#define BOARD_VCOM_LEN 16
#define BOARD_LPSTAT_LEN 16

#define MAX_IDME_VALUE_SIZE 320
#define FLAGS_LEN 20

#ifdef ENABLE_UFBL_SECURE_BOOT
#include "amzn_unlock.h"
#endif

typedef struct nvram_t {
    const char *name;
    unsigned int offset;
    unsigned int size;
} nvram_t;

static const struct nvram_t nvram_info[] = {
    {
        .name = "serial",
        .offset = 0x00,
        .size = BOARD_DSN_LEN,
    },
    {
        .name = "mac",
        .offset = 0x30,
        .size = BOARD_MAC_LEN,
    },
    {
        .name = "sec",
        .offset = 0x40,
        .size = BOARD_SEC_LEN,
    },
    {
        .name = "pcbsn",
        .offset = 0x60,
        .size = BOARD_PCBA_LEN,
    },
    {
        .name = "bootmode",
        .offset = 0x1000,
        .size = BOARD_BOOTMODE_LEN,
    },
    {
        .name = "postmode",
        .offset = 0x1010,
        .size = BOARD_POSTMODE_LEN,
    },
    {
        .name = "btmac",
        .offset = 0x1040,
        .size = 12,
    },
#ifdef CONFIG_QBOOT
    {
        .name = "oldboot",
        .offset = 0x1050,
        .size = BOARD_BOOTMODE_LEN,
    },
    {
        .name = "qbcount",
        .offset = 0x1060,
        .size = BOARD_QBCOUNT_LEN,
    },
#endif
    {
        .name = "vcom",
        .offset = 0x1080,
        .size = BOARD_VCOM_LEN,
    },
    {
        .name = "fsn",
        .offset = 0x1090,
        .size = BOARD_FSN_LEN,
    },
#ifdef ENABLE_UFBL_SECURE_BOOT
    {
        .name = "unlock_code",
        .offset = 0x10A0,
        .size = SIGNED_UNLOCK_CODE_LEN,
    },
    {
        .name = "fos_flags",
        .offset = 0x11A0,
        .size = FLAGS_LEN,
    },
    {
        .name = "dev_flags",
        .offset = 0x11B4,
        .size = FLAGS_LEN,
    },
    {
        .name = "usr_flags",
        .offset = 0x11C8,
        .size = FLAGS_LEN,
    },
#endif
    {
        .name = "lpstat",
        .offset = 0x11DC,
        .size = BOARD_LPSTAT_LEN,
    },
};

#define CONFIG_NUM_NV_VARS (sizeof(nvram_info)/sizeof(nvram_info[0]))

int idme_check_update(void);
int idme_clear_update(void);
int idme_get_var(const char *name, char *buf, int buflen);
int idme_get_var_ex(const char *name, char *buf, int buflen, bool read_all);
int idme_update_var(const char *name, const char *value);
int idme_update_var_with_size(const char *name, const char *value, int val_size);
#endif /* __IDME_H__ */
