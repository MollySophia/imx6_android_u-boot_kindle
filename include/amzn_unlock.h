#ifndef AMZN_UNLOCK_H
#define AMZN_UNLOCK_H

#define UNLOCK_CODE_LEN		(32)
#define SIGNED_UNLOCK_CODE_LEN	(256)

int amzn_get_unlock_code(unsigned char *code, unsigned int *len);
int amzn_write_unlock_code(const void *code, unsigned int len);
int amzn_verify_unlock(void *data, unsigned int size);
int amzn_device_is_unlocked(void);
int amzn_relock_target(void);

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif

#ifndef NULL
#define NULL 0
#endif


#endif
