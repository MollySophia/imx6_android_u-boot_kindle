/*
 * Copyright (C) 2015 - 2019 Amazon.com Inc. or its affiliates.  All Rights Reserved.
 */

#include <tomcrypt.h>
#include <debug.h>
#include <amzn_unlock.h>
#include "ufbl_debug.h" /* for dprintf */
#include "amzn_ufbl_alloc.h" /* for plat_alloc */

static int verify_code(void *data, unsigned int size, uint8_t *digest,
		       unsigned long digest_len, rsa_key *unlock_key,
		       unsigned int ctr_min, unsigned int ctr_max)
{
	int rc = -1, ret = -1, status = -1;
	uint8_t code[UNLOCK_CODE_LEN];
	unsigned int code_len = sizeof(code);
	unsigned int ctr;
	int sha256_idx = find_hash("sha256");

	for (ctr = ctr_min; (ctr <= ctr_max) && (status < 1); ctr++) {
		/* Retrieve unlock code and compute the digest */
		if (ctr_min != ctr_max)
			ret = amzn_get_limited_unlock_code(code, &code_len, ctr);
		else
			ret = amzn_get_unlock_code(code, &code_len);

		if (ret != 0) {
			dprintf(CRITICAL, "%s: Failed to get unlock code\n",
				__FUNCTION__);
			goto done;
		}

		hash_memory(sha256_idx, code, code_len, digest,
			    &digest_len);

		/* PSS-Verify */
		ret = rsa_verify_hash(data, size, digest, digest_len,
				      sha256_idx, sha256_desc.hashsize,
				      &status, unlock_key);
	}
	if (ret == 0 && status == 1)
		rc = 0;
done:
	return rc;
}

int amzn_verify_limited_unlock(void *data, unsigned int size, unsigned int ctr_min, unsigned int ctr_max)
{
	int rc = -1;
	unsigned int key_len = 0;
	const uint8_t *key = amzn_get_unlock_key(&key_len);
	rsa_key *unlock_key = NULL;
	uint8_t digest[sha256_desc.hashsize];
	unsigned long digest_len = sha256_desc.hashsize;

	if (!data || !key || !key_len) {
		dprintf(CRITICAL, "%s: Invalid arg\n", __FUNCTION__);
		goto done;
	}

	if (!(unlock_key = (rsa_key *)amzn_plat_alloc(sizeof(*unlock_key)))) {
		dprintf(CRITICAL, "%s: Cannot malloc key\n", __FUNCTION__);
		goto done;
	}

#ifdef USE_LTM
	ltc_mp = ltm_desc;
#elif defined(USE_TFM)
	ltc_mp = tfm_desc;
#elif defined(USE_GMP)
	ltc_mp = gmp_desc;
#else
	extern ltc_math_descriptor EXT_MATH_LIB;
	ltc_mp = EXT_MATH_LIB;
#endif

	/* Parse the unlock key */
	if (rsa_import(key, key_len, unlock_key) != CRYPT_OK) {
		dprintf(CRITICAL, "%s: Cannot parse key\n", __FUNCTION__);
		goto done;
	}

	register_hash(&sha256_desc);

	rc = verify_code(data, size, digest, digest_len, unlock_key,
			 ctr_min, ctr_max);
done:
	if (unlock_key) {
		rsa_free(unlock_key);
		amzn_plat_free(unlock_key);
	}

	return rc;
}

int amzn_verify_unlock(void *data, unsigned int size)
{
	return amzn_verify_limited_unlock(data, size, 0, 0);
}
