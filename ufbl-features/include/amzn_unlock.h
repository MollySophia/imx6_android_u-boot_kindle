/*
 * Copyright (C) 2015 - 2019 Amazon.com Inc. or its affiliates.  All Rights Reserved.
 */
#ifndef AMZN_UNLOCK_H
#define AMZN_UNLOCK_H

#define UNLOCK_CODE_LEN		(32)
#define SIGNED_UNLOCK_CODE_LEN	(256)

int amzn_verify_unlock(void *data, unsigned int size);
int amzn_target_is_unlocked(void);
int amzn_get_unlock_code(unsigned char *code, unsigned int *len);
const unsigned char *amzn_get_unlock_key(unsigned int *key_len);

/* Support for boot count limited unlock codes */
int amzn_verify_limited_unlock(void *data, unsigned int size, unsigned int ctr_min, unsigned int ctr_max);
int amzn_get_limited_unlock_code(unsigned char *code, unsigned int *len, unsigned int ctr);

#endif
