ifneq ($(filter ABC ABC,$(UFBL_PROJECT)),)
ccflags-y += -I$(UFBL_PATH)/include -DSUPPORT_UBOOT -I$(UFBL_PATH)/features/idme/include

obj-y += cmd_idme.o
obj-y += cmd_idme_v2_0.o
obj-y += idme_platform_imx.o

else
include $(TOPDIR)/config.mk

IDME_LIB = $(obj)libidme.o

SOBJS	=

CFLAGS += -I$(UFBL_PATH)/include -DSUPPORT_UBOOT -I$(UFBL_PATH)/features/idme/include

ifeq (yes,$(strip $(UFBL_PROJ_ABC)))
CFLAGS += -DUFBL_PROJ_ABC
endif


# Define IDME
CFLAGS += -DUFBL_FEATURE_IDME
ifeq (yes,$(strip $(UFBL_PROJ_ABC)))
COBJS-y += cmd_idme.o cmd_idme_v2_0.o idme_platform_mstar.o
else ifeq (yes,$(strip $(UFBL_PROJ_ABC)))
COBJS-y += cmd_idme.o cmd_idme_v2_0.o idme_platform_mstar.o
else
COBJS-y += cmd_idme.o cmd_idme_v2_0.o idme_platform_brcm.o
endif
COBJS	= $(COBJS-y)
SRCS 	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

$(IDME_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
#include $(SRCTREE)/rules.mk

# sinclude $(obj).depend

#########################################################################
endif
