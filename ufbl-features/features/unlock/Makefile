# Copyright (c) 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved

#
# Common list of objects files we want to include
#
UNLOCK_OBJECTS = unlock.o

# Common set of CFLAGS we need
UNLOCK_CFLAGS = \
	-DSUPPORT_UBOOT \
	-std=c99 \
	-ffunction-sections -fdata-sections \
	-DUFBL_FEATURE_UNLOCK

# Additional objects and CFLAGS depending on which
# crypto library we can using
ifeq (true,$(strip $(FEATURE_COMMON_OPENSSL)))
UNLOCK_OBJECTS += unlock_openssl.o
UNLOCK_CFLAGS += -DUFBL_FEATURE_UNLOCK_OPENSSL
endif

ifeq (true,$(strip $(FEATURE_LIBTOMCRYPT)))
UNLOCK_OBJECTS += unlock_ltc.o

UNLOCK_HEADERS = \
	-I$(UFBL_PATH)/features/libtomcrypt/ \
	-I$(UFBL_PATH)/features/libtomcrypt/src/headers

UNLOCK_CFLAGS += \
	-DUFBL_FEATURE_UNLOCK_LTC \
	-DLTC_SOURCE \
	-DLTC_NO_FILE \
	-DUSE_LTM -DLTM_DESC \
	-D_WCHAR_T -DLTC_CLEAN_STACK
endif

# Begin u-boot specific rules
ifeq ($(UFBL_CONFIG_KBUILD),true)

UBOOTINCLUDE += $(UNLOCK_HEADERS)
ccflags-y    += $(UNLOCK_CFLAGS)
obj-y        += $(UNLOCK_OBJECTS)

else # default rules


include $(TOPDIR)/config.mk

UNLOCK_LIB = $(obj)libunlock.o
SOBJS   =

# Default CFLAGS
CFLAGS += \
	$(UNLOCK_CFLAGS) \
	$(UNLOCK_HEADERS)

COBJS-y += \
	$(UNLOCK_OBJECTS)

COBJS   = $(COBJS-y)
SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS) $(COBJS))

$(UNLOCK_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif #UFBL_CONFIG_KBUILD
