#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/../../include
INCLUDES += -I$(LOCAL_DIR)/include

# Define FEATURE_LIFE_CYCLE_REASONS
CFLAGS += -DUFBL_FEATURE_LIFE_CYCLE_REASONS

OBJS += \
    $(LOCAL_DIR)/life_cycle_reasons.o \

ifeq ($(UFBL_PROJECT),$(filter $(UFBL_PROJECT),ABC ABC_proto ABC ABC ABC ABC ABC ABC ABC))
OBJS += \
    $(LOCAL_DIR)/life_cycle_reasons_mtk.o
endif

ifeq ($(UFBL_PROJECT),$(filter $(UFBL_PROJECT),ABC))
OBJS += \
    $(LOCAL_DIR)/life_cycle_reasons_mtk_dummypmic.o
endif
