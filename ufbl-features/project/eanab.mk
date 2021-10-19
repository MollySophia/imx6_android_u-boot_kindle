LOCAL_DIR := $(UFBL_PATH)/project

# List of features for this product
FEATURE_LIBTOMCRYPT := true
FEATURE_LIBTOMMATH := true
FEATURE_SECURE_BOOT := true
FEATURE_UNLOCK := true
FEATURE_FOS_FLAGS := true

KBUILD_CFLAGS += -DNON_ANCIENT_UBOOT_LK

export FEATURE_LIBTOMCRYPT FEATURE_LIBTOMMATH FEATURE_SECURE_BOOT FEATURE_UNLOCK

# After defining list of features, include
# ufbl common definitions
include $(LOCAL_DIR)/ufbl_common_uboot.mk
