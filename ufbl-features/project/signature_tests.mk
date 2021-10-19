#Copyright (C) 2017 Amazon.com Inc. or its affiliates.  All Rights Reserved

LOCAL_DIR := $(GET_LOCAL_DIR)
# List of features for this product
FEATURE_SECURE_BOOT := true

DEFINES += UFBL_SIG_TESTS
ifeq (libtomcrypt, $(MAKECMDGOALS))
# This is required for fastboot extensions
DEFINES += UFBL_FEATURE_SECURE_BOOT_LTC
DEFINES += LTC_NO_FILE
DEFINES += USE_LTM
DEFINES += LTM_DESC
DEFINES += LTC_CLEAN_STACK
DEFINES += LTM_NO_FILE
OBJS += \
    $(LOCAL_DIR)/../features/secure_boot/image_verify_ltc.o
else
# This is required for fastboot extensions
DEFINES += FEATURE_COMMON_OPENSSL
DEFINES += UFBL_FEATURE_SECURE_BOOT_OPENSSL
OBJS += \
    $(LOCAL_DIR)/../features/secure_boot/image_verify_openssl.o
endif
# After defining list of features, include
# ufbl common definitions
include $(LOCAL_DIR)/ufbl_common.mk
