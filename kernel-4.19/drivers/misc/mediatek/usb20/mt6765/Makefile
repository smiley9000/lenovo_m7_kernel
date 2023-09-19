#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#
#Liverpool code for AX3207-366 by liruiju at 20-11-05 start
subdir-ccflags-y += -I$(srctree)/drivers/iio/
#Liverpool code for AX3207-366 by liruiju at 20-11-05 end
ccflags-$(CONFIG_USB_MTK_HDRC) += -I$(srctree)/drivers/misc/mediatek/usb20

# for battery related
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat

# for SPM control usage
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/

# for TYPEC connection management
ccflags-y += -I$(srctree)/drivers/misc/mediatek/typec/inc

#Liverpool code for AX3207-366 by liruiju at 20-11-05 start
ccflags-y += -I$(srctree)/drivers/power/supply
ccflags-y += -I$(srctree)/drivers/misc/mediatek/usb20/mt6765/
#Liverpool code for AX3207-366 by liruiju at 20-11-05 end

ifeq ($(CONFIG_TCPC_CLASS),y)
	ccflags-y += -I$(srctree)/drivers/misc/mediatek/typec/tcpc/inc
endif
# for ep0 test
ccflags-y += -I$(srctree)/drivers/usb/core/
obj-$(CONFIG_USB_MTK_HDRC)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_MTK_HDRC)  += usb20_host.o
obj-$(CONFIG_USBIF_COMPLIANCE)   += usb20_otg_if.o
obj-$(CONFIG_DEBUG_FS)	+= usb20_phy_debugfs.o
#Liverpool code for AX3207-366 by liruiju at 20-11-05 start
obj-$(CONFIG_CHARGER_THUB_ADC)  += thub_adc.o
#Liverpool code for AX3207-366 by liruiju at 20-11-05 end