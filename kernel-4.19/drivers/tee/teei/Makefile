# Copyright (c) 2015-2020, MICROTRUST Incorporated
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# Liverpool code for AX3207-292 by yanwenlong at 20201106 start
ifeq ($(CONFIG_MICROTRUST_TEE_LITE_SUPPORT), y)
$(info "CONFIG_MICROTRUST_TEE_LITE_SUPPORT=$(CONFIG_MICROTRUST_TEE_LITE_SUPPORT)")
obj-y += lite/
endif
# Liverpool code for AX3207-292 by yanwenlong at 20201106 end

ifeq ($(CONFIG_MICROTRUST_TEE_SUPPORT), y)
$(info "CONFIG_MICROTRUST_TEE_VERSION=$(CONFIG_MICROTRUST_TEE_VERSION)")
obj-y += $(subst ",,$(CONFIG_MICROTRUST_TEE_VERSION))/
endif
