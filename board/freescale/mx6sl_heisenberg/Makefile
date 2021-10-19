# (C) Copyright 2013 Freescale Semiconductor, Inc.
# (C) Copyright 2015 Amazon.com, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y  := mx6sl_heisenberg.o
obj-y  += pinmux.o
#obj-$(CONFIG_LP5523_LED_CONTROL)  += led_control.o
obj-$(CONFIG_PMIC_MAX77696) += pmic.o
obj-$(CONFIG_PMIC_ROHM) += pmic_rohm.o

# UFBL Feature
ifeq ($(ENABLE_UFBL_SECURE_BOOT),true)
obj-y += secure_boot_cfg.o
obj-y += eanab_unlock.o
endif #ENABLE_UFBL_SECURE_BOOT

extra-$(CONFIG_USE_PLUGIN) :=  plugin.bin
$(obj)/plugin.bin: $(obj)/plugin.o
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@
