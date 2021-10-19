# Copyright (c) 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved

#
# Common list of objects files we want to include
#
FOS_FLAGS_OBJECTS = \
	fos_flags.o \
	fastboot_fos_flags.o

ifeq ($(UFBL_CONFIG_KBUILD),true)

obj-y += $(FOS_FLAGS_OBJECTS)

else # default rules

FOS_FLAGS_LIB = $(obj)libfos_flags.o
SOBJS   =

COBJS-y += \
	$(FOS_FLAGS_OBJECTS)

COBJS   = $(COBJS-y)
SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS) $(COBJS))

$(FOS_FLAGS_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif #UFBL_CONFIG_KBUILD
