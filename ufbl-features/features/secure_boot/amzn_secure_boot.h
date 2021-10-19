/*
 * Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
 */
#ifndef __AMZN_SECURE_BOOT_H
#define __AMZN_SECURE_BOOT_H

#include <stddef.h> /* size_t */

enum {
	AMZN_ENGINEERING_DEVICE = 0,
	AMZN_PRODUCTION_DEVICE,
	AMZN_INVALID_DEVICE,
};

enum {
	AMZN_ENGINEERING_CERT = 0,
	AMZN_PRODUCTION_CERT,
};

/* Define callback type for meta-data */
typedef int (*meta_data_handler)(const char *meta_data);

int amzn_image_verify(const void *image,
	unsigned char *signature,
	unsigned int image_size, meta_data_handler handler);

enum safe_verify_response {
	/* The image is unsigned and we have no reason to load it */
	AMZN_IMAGE_BAD = 0,
	/* The image is production signed */
	AMZN_IMAGE_SIGNED = 1 << 0,
	/* The image is development signed */
	AMZN_IMAGE_DEVSIGNED = 1 << 1,
	/* The image is development signed, and the device is unlocked */
	AMZN_IMAGE_DEVSIGNED_UNLOCKED = 1 << 2,
	/* The image is dev signed, and the device is an engineering device */
	AMZN_IMAGE_DEVSIGNED_ENG = 1 << 3,
	/* The image is not signed, but the device is unlocked */
	AMZN_IMAGE_UNSIGNED_UNLOCKED = 1 << 4,
	/* The image is not signed, but the device is an engineering device */
	AMZN_IMAGE_UNSIGNED_ENG = 1 << 5,
};

#ifdef __cplusplus
inline safe_verify_response operator|
	(safe_verify_response a, safe_verify_response b)
{
	return static_cast < safe_verify_response >
		(static_cast < int > (a) | static_cast < int > (b));
}
#endif

enum safe_verify_response amzn_image_verify_safe(const void *image,
	unsigned char *signature,
	unsigned int image_size, meta_data_handler handler);

/* Returns engineering or production certificate */
const unsigned char *amzn_get_kernel_cert(int cert_type, size_t *length);

/* Implementation-specific weak functions */
int amzn_target_device_type(void);
const char *amzn_target_device_name(void);

void amzn_target_sha256(const void *data, size_t n, void *digest);
const unsigned char *amzn_get_kernel_cert(int cert_type, size_t *length);

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif

#endif
