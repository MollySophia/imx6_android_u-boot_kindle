/*
 * Copyright (C) 2016 - 2019 Amazon.com Inc. or its affiliates.  All Rights Reserved.
 */

#include "common.h"
#include "fastboot.h"
#include "fos_flags.h"

bool fastboot_oem_flags(char *arg, /* out */ char *response) {
	/* 'arg' is a null-terminated string that is the full set of arguments, less 'oem '. */
	arg += strlen("flags ");
	bool success = false;

	snprintf(response, FASTBOOT_RESPONSE_LEN, "INFO");
	success = oem_flags(arg, response + 4, FASTBOOT_RESPONSE_LEN - 4);
	return success;
}
