#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
# Include rules for each feature
FEATURE_DIR := $(call my-dir)

UFBL_SOURCE_FILES :=
UFBL_INCLUDE_FILES := include
UFBL_CFLAGS :=

ifeq (true,$(strip $(FEATURE_SECURE_BOOT)))
include $(FEATURE_DIR)/secure_boot/rules_android.mk
endif

ifeq (true,$(strip $(FEATURE_UNLOCK)))
include $(FEATURE_DIR)/unlock/rules_android.mk
endif
