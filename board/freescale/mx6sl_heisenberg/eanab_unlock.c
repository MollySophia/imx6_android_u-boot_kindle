#include "amzn_unlock.h"
#include "amzn_secure_boot.h"
#include <common.h>
#include <command.h>
#include <fuse.h>
#include <asm/errno.h>
#include <lab126/idme.h>
#include <malloc.h>

int amzn_get_unlock_code(unsigned char *code, unsigned int *len)
{
	if (!code || !len || *len < (18 + 1))
		return -1;

	u32 val_low;
	u32 val_high;
	int ret;

	// Get Device Unique ID - Fuse OTP Bank 0 - Word 1
	ret = fuse_read(0, 1, &val_low);
	if (ret != 0) {
		printf ("Error reading SoC Unique ID (B0W1)\n");
		return ret;
	}

	// Get Device Unique ID - Fuse OTP Bank 0 - Word 2
	ret = fuse_read(0, 2, &val_high);
	if (ret != 0) {
		printf ("Error reading SoC Unique ID (B0W2)\n");
		return ret;
	}

	*len = snprintf((char*)code, *len, "0x%08x%08x", val_high, val_low);
	return 0;
}

const unsigned char *amzn_get_unlock_key(unsigned int *key_len)
{
	static const unsigned char unlock_key[] =
		"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01"
		"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01"
		"\x00\xcd\x2a\xf7\x5a\xc5\xdb\xc7\xd0\x14\xef\xea\x1a\x85\x88\x39"
		"\xd6\xb1\xd7\xca\x72\x3a\x09\x9e\xd4\x5f\x6c\xc6\x0e\xd3\x33\xf0"
		"\x6c\x26\x0b\xfe\x65\x1c\x96\x1a\xed\xcd\xe8\xf4\xfb\x3c\xb4\x1b"
		"\xa0\xfb\x40\x7a\x76\x89\xf8\x02\xef\x08\x91\x62\x3a\x6c\xf2\xe9"
		"\xe4\x74\x96\xbf\x18\xd6\x62\x59\x6f\xca\xe7\x66\x58\xdd\x3e\xe8"
		"\x12\xe5\xec\xc2\xff\xa6\x66\x19\x8a\x61\xc4\x11\x45\xd1\xf9\xec"
		"\x68\x85\x24\x2c\x99\x41\xe1\xba\x1b\xb1\x62\x71\x14\x6a\x19\x86"
		"\x48\xa9\xa0\x21\x38\xb4\xda\x72\xcd\x73\xe7\x14\xc0\x6e\xd4\x9d"
		"\x79\xa8\x9d\xdc\x02\xfe\x4b\xc7\x87\x54\x22\x7d\xe7\xb8\x80\xfb"
		"\xa2\x79\xe8\x0f\x0e\x6e\x40\xac\x94\x6f\x66\xc3\x9a\x98\x0d\xdf"
		"\xa7\x05\xa5\xf1\x5f\x11\x1a\x49\x74\x3b\x5f\xdb\x9e\x5a\x07\x17"
		"\x25\x3a\x53\x68\x7c\xa9\x8e\xc9\x40\xbb\x7c\x2f\x3a\x0b\x38\x75"
		"\x6b\xd1\x68\x35\x48\x90\xbb\x87\x37\x03\x80\xc1\x87\xc4\xc5\x5f"
		"\x34\xf1\x5d\x3e\x64\x56\xac\x10\x81\xb0\x9b\x9b\xf9\x8c\xaa\x2f"
		"\x42\xe1\x5a\x76\xfa\xf5\xf9\x35\x7b\xd0\xf3\xdb\xe9\x4a\x04\x43"
		"\x29\x5c\xb6\x90\xd7\x34\xaa\x8c\x99\x08\x35\x03\x47\x3d\xfd\x8b"
		"\x23\x02\x03\x01\x00\x01"
		;

	static const int unlock_key_size = 294;

	if (!key_len)
		return NULL;

	*key_len = unlock_key_size;

	return unlock_key;
}

int amzn_write_unlock_code(const void *code, unsigned int len)
{
	int ret = -1;

	if (!code || len != SIGNED_UNLOCK_CODE_LEN) {
		printf ("Unlock code invalid len (%d)!\n", len);
		return -1;
	}

	if (amzn_verify_unlock(code, len)) {
		printf ("Unlock code invalid!\n");
		return -1;
	}

	// Write the data into IDME
	ret = idme_update_var_with_size("unlock_code", code, len);
	if (ret > 0) {
		printf ("Unlock code written in IDME (%d)!\n", ret);
		ret = 0; // success
	}
	else {
		printf("Failed to write unlock code into IDME\n");
		ret = -1;
	}

	return ret;
}

int amzn_relock_target(void)
{
	int ret = -1;
	unsigned char *code = (unsigned char *)malloc(SIGNED_UNLOCK_CODE_LEN);
	if (!code)
		return ret;

	memset(code, 0, SIGNED_UNLOCK_CODE_LEN);

	// Write empty data into IDME
	if (idme_update_var_with_size("unlock_code", code, SIGNED_UNLOCK_CODE_LEN) > 0) {
		printf ("Erased code in IDME!\n");
		ret = 0;
	}
	else {
		printf("Failed to erase unlock code into IDME\n");
	}

	free (code);
	return ret;
}

int amzn_device_is_unlocked(void)
{
	int ret = 0;
	unsigned char *signed_code = (unsigned char *)malloc(SIGNED_UNLOCK_CODE_LEN + 1);
	if (!signed_code)
		return ret;

	if (idme_get_var("unlock_code", signed_code, SIGNED_UNLOCK_CODE_LEN + 1)!=0) {
		printf ("Failed to get Signed Unlock Code!\n");
		goto end;
	}

	if (!(amzn_verify_unlock(signed_code, SIGNED_UNLOCK_CODE_LEN)))
	{
		printf ("Verify Signed Unlock Code Successfully!\n");
		ret = 1;
		goto end;
	}
	else
	{
		printf ("Verify Signed Unlock Code Failed!\n");
	}

end:
	if (signed_code)
		free (signed_code);

	if (!ret && amzn_target_device_type()==AMZN_ENGINEERING_DEVICE) {
		printf ("Force unlock for engineering device!\n");
		return 1;
	}

	return ret;
}
