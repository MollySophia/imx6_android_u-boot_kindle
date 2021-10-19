#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
#subdir-ccflags-$(CONFIG_ENABLE_IDME) += -DUFBL_FEATURE_IDME

subdir-ccflags-$(CONFIG_ENABLE_IDME) += \
-I$(srctree)/../../ufbl-features/features/libtomcrypt/uboot \
-DSUPPORT_UBOOT
subdir-ccflags-$(CONFIG_ENABLE_IDME) += -I$(srctree)/../../ufbl-features/features/idme/include


# Leave line above blank
obj-$(CONFIG_ENABLE_IDME) += \
../../../../ufbl-features/features/idme/cmd_idme.o \
../../../../ufbl-features/features/idme/cmd_idme_v2_0.o \

# Leave line above blank
ifeq (yes,$(strip $(UFBL_PROJ_ABC)$(UFBL_PROJ_ABC)))
obj-$(CONFIG_ENABLE_IDME) += ../../../../ufbl-features/features/idme/idme_platform_ABC.o
else
obj-$(CONFIG_ENABLE_IDME) += ../../../../ufbl-features/features/idme/idme_platform.o
endif
