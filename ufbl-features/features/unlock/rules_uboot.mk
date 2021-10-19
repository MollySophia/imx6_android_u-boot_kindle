#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
ifdef CONFIG_SECURE_UNLOCK
subdir-ccflags-y += -DUFBL_FEATURE_UNLOCK
subdir-ccflags-$(CONFIG_LIBTOMCRYPT) += -DUFBL_FEATURE_UNLOCK_LTC \
-I$(srctree)/../../ufbl-features/features/libtomcrypt/src/headers \
-I$(srctree)/../../ufbl-features/features/libtomcrypt \
-I$(srctree)/../../ufbl-features/features/libtomcrypt/uboot \
-DLTC_SOURCE \
-DLTC_NO_FILE \
-DUSE_LTM -DLTM_DESC \
-D_WCHAR_T -DLTC_CLEAN_STACK \
-DSUPPORT_UBOOT

# Leave line above blank
obj-y += ../../../../ufbl-features/features/unlock/unlock.o
obj-$(CONFIG_LIBTOMCRYPT) += ../../../../ufbl-features/features/unlock/unlock_ltc.o
obj-$(CONFIG_LIBPOLARSSL) += ../../../../ufbl-features/features/unlock/unlock_polarssl.o
endif
