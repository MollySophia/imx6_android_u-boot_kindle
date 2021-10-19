#include "amzn_secure_boot.h"
#include <common.h>
#include <fuse.h>

const char *amzn_target_device_name(void)
{
	return "eanab";
}

int amzn_target_device_type(void)
{
	u32 val;
	int ret;

	// Production bit:
	//	Use Fuse OTP Bank 4 - Word 6 (OCOTP_GP1)
	ret = fuse_read(4, 6, &val);
	if (ret != 0)
		return AMZN_INVALID_DEVICE;

	if (val & 0x1)
		return AMZN_PRODUCTION_DEVICE;

	return AMZN_ENGINEERING_DEVICE;
}
