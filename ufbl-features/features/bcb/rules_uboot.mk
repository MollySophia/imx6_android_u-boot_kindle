#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ENABLE_BCB_SPL) += ../../../../ufbl-features/features/bcb/bcb.o
else
obj-$(CONFIG_ENABLE_BCB) += ../../../../ufbl-features/features/bcb/bcb.o
obj-$(CONFIG_ENABLE_BCB) += ../../../../ufbl-features/features/bcb/cmd_bcb.o
endif
