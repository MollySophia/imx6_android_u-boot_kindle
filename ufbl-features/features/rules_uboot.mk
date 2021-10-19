#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
# Include rules for each feature
FEATURE_DIR = ufbl-features/features
UFBL_LIBS =

KBUILD_CFLAGS += -DSUPPORT_UBOOT
UBOOTINCLUDE += -I$(UFBL_PATH)/include

ifeq (true,$(strip $(FEATURE_IDME)))
UFBL_LIBS += $(FEATURE_DIR)/idme/libidme.o
endif

ifeq (true,$(strip $(FEATURE_COMMON_OPENSSL)))
UFBL_LIBS += $(FEATURE_DIR)/common_openssl/libopenssl.o
endif

ifeq (true,$(strip $(FEATURE_SECURE_BOOT)))
UFBL_LIBS += $(FEATURE_DIR)/secure_boot/libsecure_boot.o
endif

ifeq (true,$(strip $(FEATURE_UNLOCK)))
UFBL_LIBS += $(FEATURE_DIR)/unlock/libunlock.o
endif

ifeq (true,$(strip $(FEATURE_LIBTOMMATH)))
UFBL_LIBS += $(FEATURE_DIR)/libtommath/libtommath.o
endif

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
UFBL_LIBS += $(FEATURE_DIR)/libtomcrypt/libtomcrypt.o
endif

ifeq (true,$(strip $(FEATURE_FOS_FLAGS)))
UFBL_LIBS += $(FEATURE_DIR)/fos_flags/libfos_flags.o
endif
