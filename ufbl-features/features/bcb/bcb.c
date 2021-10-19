/*
 * Copyright (C) 2016 - 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */

#ifndef SUPPORT_UBOOT
#include <string.h>
#include <rand.h>
#include <debug.h>
#else
#include <common.h>
#include "ufbl_debug.h"
#endif
#include "bcb.h"
#include "bcb_platform.h"
#include "bootctrl.h"

static bool ufbl_read_misc(char *misc_buf, int size)
{
    int retry = 3;
    while(retry --) {
        if(true == platform_read_partition("misc", misc_buf, size))
            return true;
    }

    return false;
}

static bool ufbl_write_misc(char *misc_buf, int size)
{
    int retry = 3;
    while(retry --) {
        if(true == platform_write_partition("misc", misc_buf, size))
            return true;
    }
    return false;
}

static void create_default_metadata(boot_ctrl_t *bctrl, int default_active_slot)
{
    dprintf(INFO, "create_default_metadata, default_active_slot = %d\n", default_active_slot);

    bctrl->magic = BOOTCTRL_MAGIC;
    bctrl->version = BOOT_CONTROL_VERSION;

    if(default_active_slot == 0) {
        bctrl->slot_info[0].priority = BCB_DEFAULT_ACTIVE_SLOT_PRIORITY;
        bctrl->slot_info[1].priority = BCB_DEFAULT_OTHER_SLOT_PRIORITY;
    }
    else {
        bctrl->slot_info[0].priority = BCB_DEFAULT_OTHER_SLOT_PRIORITY;
        bctrl->slot_info[1].priority = BCB_DEFAULT_ACTIVE_SLOT_PRIORITY;
    }

    bctrl->slot_info[0].tries_remaining = BCB_DEFAULT_MAX_RETRIES;
    bctrl->slot_info[0].successful_boot = 0;

    bctrl->slot_info[1].tries_remaining = BCB_DEFAULT_MAX_RETRIES;
    bctrl->slot_info[1].successful_boot = 0;
}

bool ufbl_get_boot_ctrl(boot_ctrl_t *bctrl)
{
    bool ret = false;
    char misc_buf[OFFSETOF_SLOT_SUFFIX + sizeof(boot_ctrl_t)];
    boot_ctrl_t *bctrl_read = (boot_ctrl_t *)&misc_buf[OFFSETOF_SLOT_SUFFIX];

    if(platform_support_pingpong() && bctrl) {
        if(ufbl_read_misc(misc_buf, sizeof(misc_buf))) {
            memcpy(bctrl, bctrl_read, sizeof(boot_ctrl_t));
            ret = true;
        }
    }

    return ret;
}

bool ufbl_set_boot_ctrl(const boot_ctrl_t *bctrl)
{
    bool ret = false;
    char misc_buf[OFFSETOF_SLOT_SUFFIX + sizeof(boot_ctrl_t)];
    boot_ctrl_t *bctrl_write = (boot_ctrl_t *)&misc_buf[OFFSETOF_SLOT_SUFFIX];

    if(platform_support_pingpong() && bctrl) {
        memcpy(bctrl_write, bctrl, sizeof(boot_ctrl_t));
        if(ufbl_write_misc(misc_buf, sizeof(misc_buf))) {
            ret = true;
        }
    }

    return ret;
}

int ufbl_get_active_slot(void)
{
    char misc_buf[OFFSETOF_SLOT_SUFFIX + sizeof(boot_ctrl_t)];
    boot_ctrl_t *bctrl = (boot_ctrl_t *)&misc_buf[OFFSETOF_SLOT_SUFFIX];
    int active_slot = -1;
    bool write_back = false;
    int default_slot = 0;
#ifdef CONFIG_BCB_RANDOMISE_DEFAULT
    default_slot = rand() % 2;
#endif

    if(false == platform_support_pingpong())
        return active_slot;

    memset(misc_buf, 0, sizeof(misc_buf));

    // read BCB
    if(false == ufbl_read_misc(misc_buf, sizeof(misc_buf))) {
        memset(misc_buf, 0, sizeof(misc_buf));

        create_default_metadata(bctrl, default_slot);
        write_back = true;
    }
    else {
        dprintf(INFO, "read misc: %06x %08x %08x\n", OFFSETOF_SLOT_SUFFIX, *(int *)bctrl, *((int *)bctrl + 1));
    }

    // sanity check
    if(bctrl->magic != BOOTCTRL_MAGIC) {
        dprintf(CRITICAL, "metadata is not initialized or corrupted.\n");
        memset(misc_buf, 0, sizeof(misc_buf));
        create_default_metadata(bctrl, default_slot);
        write_back = true;
    }

    if(bctrl->slot_info[0].priority > bctrl->slot_info[1].priority)
        active_slot = 0;
    else if(bctrl->slot_info[0].priority < bctrl->slot_info[1].priority)
        active_slot = 1;
    else {
        if(bctrl->slot_info[0].priority > 0) {
            memset(misc_buf, 0, sizeof(misc_buf));
            create_default_metadata(bctrl, default_slot);
            active_slot = default_slot;
            write_back = true;
        }
        else {
            // No bootable slots
            active_slot = -1;
        }
    }

    if(active_slot >= 0) {
        if(platform_last_boot_failed_pingpong()) {
            if(bctrl->slot_info[active_slot].tries_remaining > 0) {
                bctrl->slot_info[active_slot].tries_remaining --;
            }
            else {
                bctrl->slot_info[active_slot].priority = 0;
                bctrl->slot_info[active_slot].successful_boot = 0;
                active_slot = (active_slot == 0) ? 1 : 0;
                if(bctrl->slot_info[active_slot].priority == 0) {
                    // No bootable slots.  Mark as such and continue to
                    // allow writeback to BCB
                    active_slot = -1;
                }
            }
            write_back = true;
        }
    }

    if(write_back) {
        dprintf(INFO, "write misc: %06x %08x %08x\n", OFFSETOF_SLOT_SUFFIX, *(int *)bctrl, *((int *)bctrl + 1));

        if(true == ufbl_write_misc(misc_buf, sizeof(misc_buf))) {
            // for debug purpose
            memset(misc_buf, 0, sizeof(misc_buf));
            ufbl_read_misc(misc_buf, sizeof(misc_buf));
            dprintf(SPEW, "readback misc: %06x %08x %08x\n", OFFSETOF_SLOT_SUFFIX, *(int *)bctrl, *((int *)bctrl + 1));
        }
        else {
            // ignore error and continue to boot
        }
    }

#ifndef CONFIG_BCB_RETURN_FAIL_ON_RETRY_LIMIT
    if (active_slot < 0) {
        dprintf(CRITICAL, "No bootable slots!\n");
        while(1);  // Hang
    }
#endif
    dprintf(INFO, "active_slot is %d\n", active_slot);

    // Otherwise, caller handles invalid active slot
    return active_slot;
}

bool ufbl_is_slot_bootable(int slot)
{
    bool bootable = false;
    boot_ctrl_t bctrl;

    if (ufbl_get_boot_ctrl(&bctrl)) {
        if (bctrl.slot_info[slot].priority > 0)
            bootable = true;
    }

    return bootable;
}

int ufbl_set_slot_as_unbootable(int slot)
{
    int ret = -1;
    boot_ctrl_t bctrl;
    if (ufbl_get_boot_ctrl(&bctrl)) {
        bctrl.slot_info[slot].tries_remaining = 0;
        bctrl.slot_info[slot].priority = 0;
        bctrl.slot_info[slot].successful_boot = 0;
        if (ufbl_set_boot_ctrl(&bctrl))
            ret = 0;
    }

    return ret;
}
