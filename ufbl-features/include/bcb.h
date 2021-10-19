/*
 * Copyright (C) 2016 - 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */

#ifndef __UFBL_BOOT_H
#define __UFBL_BOOT_H

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

/** ufbl_get_active_slot
 *
 * Determine the currently active slot by querying the BCB.
 *
 * @return The current active slot (0 or 1) or -1 for failure
 */
int ufbl_get_active_slot(void);

/** ufbl_is_slot_bootable
 *
 * Query the BCB to determine if the given slot is bootable
 *
 * @param slot - The slot under consideration
 * @return true if the slot is bootable. false if not or there is an error
 *         accessing the BCB
 */
bool ufbl_is_slot_bootable(int slot);

/** ufbl_set_slot_as_unbootable
 *
 * Indicate that the slot has failed to boot.  This will mark the slot as
 * unbootable.
 *
 * @param slot - The slot to mark as failed
 * @return 0 for success or -1 for a bcb read/write error.
 */
int ufbl_set_slot_as_unbootable(int slot);

#define BCB_DEFAULT_MAX_RETRIES 3
#define BCB_MAX_SLOT_PRIORITY 15
#define BCB_DEFAULT_ACTIVE_SLOT_PRIORITY BCB_MAX_SLOT_PRIORITY
#define BCB_DEFAULT_OTHER_SLOT_PRIORITY (BCB_MAX_SLOT_PRIORITY - 1)
/* The code requires active slot priority > other slot priority.  Verify this */
STATIC_ASSERT(BCB_DEFAULT_ACTIVE_SLOT_PRIORITY > BCB_DEFAULT_OTHER_SLOT_PRIORITY, check_slot_priority);
#endif

