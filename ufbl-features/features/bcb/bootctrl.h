/*
 * Copyright (C) 2016 - 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */
/* THE HAL BOOTCTRL HEADER MUST BE IN SYNC WITH THE UBOOT BOOTCTRL HEADER */

#ifndef _BOOTCTRL_H_
#define _BOOTCTRL_H_

#ifndef SUPPORT_UBOOT
#include <stdint.h>
#endif

/* struct boot_ctrl occupies the slot_suffix field of
 * struct bootloader_message */
#define OFFSETOF_SLOT_SUFFIX 864

#define BOOTCTRL_MAGIC 0x42424100
#define BOOTCTRL_SUFFIX_A           "_a"
#define BOOTCTRL_SUFFIX_B           "_b"

#define BOOT_CONTROL_VERSION    1

typedef struct slot_metadata {
    uint8_t priority : 4;
    uint8_t tries_remaining : 3;
    uint8_t successful_boot : 1;
} slot_metadata_t;

typedef struct boot_ctrl {
    /* Magic for identification - '\0ABB' (Boot Contrl Magic) */
    uint32_t magic;

    /* Version of struct. */
    uint8_t version;

    /* Information about each slot. */
    slot_metadata_t slot_info[2];

    uint8_t recovery_tries_remaining;
} boot_ctrl_t;

bool ufbl_set_boot_ctrl(const boot_ctrl_t *bctrl);
bool ufbl_get_boot_ctrl(boot_ctrl_t *bctrl);

#endif /* _BOOTCTRL_H_ */
