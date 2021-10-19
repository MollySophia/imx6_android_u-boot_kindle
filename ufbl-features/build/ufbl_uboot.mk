#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
#UFBL setting
ENABLE_UFBL_FEATURES := 1
ifeq ($(ENABLE_UFBL_FEATURES),1)
UFBL_FEATURES_ROOT := $(UFBL_PATH)
endif

ifeq ($(ENABLE_UFBL_FEATURES),1)
UFBL_INCLUDES = -I$(UFBL_FEATURES_ROOT)/include
UFBL_CFLAGS = -DCONFIG_UFBL

ifeq ($(UFBL_CONFIG_KBUILD),true)
UBOOTINCLUDE += $(UFBL_INCLUDES)
ccflags-y   += $(UFBL_CFLAGS)
else
INCLUDES += $(UFBL_INCLUDES)
CFLAGS += $(UFBL_CFLAGS)
endif #UFBL_CONFIG_KBUILD

endif

ifeq ($(ENABLE_UFBL_FEATURES),1)
include $(UFBL_FEATURES_ROOT)/project/$(UFBL_PROJECT).mk
endif
