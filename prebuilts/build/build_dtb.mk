my_kernel_dtb :=

ifneq ($(words $(LOCAL_SRC_FILES)),1)
$(error LOCAL_SRC_FILES $(LOCAL_SRC_FILES) should be one file)
endif

ifneq (true,$(strip $(TARGET_NO_KERNEL)))
ifneq ($(LINUX_KERNEL_VERSION),)

ifeq (,$(KERNEL_OUT))
include $(LINUX_KERNEL_VERSION)/kenv.mk
endif

ifeq ($(KERNEL_TARGET_ARCH),arm64)
my_kernel_dtb_stem := mediatek/$(notdir $(patsubst %.dts,%.dtb,$(LOCAL_SRC_FILES)))
else
my_kernel_dtb_stem := $(notdir $(patsubst %.dts,%.dtb,$(LOCAL_SRC_FILES)))
endif

ifeq ($(KERNEL_TARGET_ARCH),arm64)
my_kernel_dts := $(KERNEL_OUT)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/mediatek/$(notdir $(LOCAL_SRC_FILES))
else
my_kernel_dts := $(KERNEL_OUT)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/$(notdir $(LOCAL_SRC_FILES))
endif

my_kernel_dtb := $(KERNEL_OUT)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/$(my_kernel_dtb_stem)

$(my_kernel_dts): $(LOCAL_SRC_FILES)
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $< $@

$(my_kernel_dtb): KOUT := $(KERNEL_OUT)
$(my_kernel_dtb): OPTS := $(KERNEL_MAKE_OPTION) $(my_kernel_dtb_stem)
$(my_kernel_dtb): $(KERNEL_ZIMAGE_OUT) $(my_kernel_dts) $(LOCAL_ADDITIONAL_DEPENDENCIES) | $(LOCAL_ADDITIONAL_DEPENDENCIES_D)
	$(PREBUILT_MAKE_PREFIX)$(MAKE) -C $(KOUT) $(OPTS)

UFDT_TOOL := $(LINUX_KERNEL_VERSION)/scripts/dtc/ufdt_apply_overlay
ifneq (,$(MTK_CHECK_DTBO_MAIN))
$(my_kernel_dtb).merge: private_my_platform_dtb := $(MTK_CHECK_DTBO_MAIN)
$(my_kernel_dtb).merge: private_my_kernel_dtb := $(my_kernel_dtb)
$(my_kernel_dtb).merge: $(UFDT_TOOL) $(MTK_CHECK_DTBO_MAIN) $(my_kernel_dtb)
	@echo "dtbo_check: $@"
	$(UFDT_TOOL) $(private_my_platform_dtb) $(private_my_kernel_dtb) $@
$(PRODUCT_OUT)/dtbo.img: $(my_kernel_dtb).merge
$(PRODUCT_OUT)/boot.img: $(my_kernel_dtb).merge
endif


endif#TARGET_NO_KERNEL
endif#LINUX_KERNEL_VERSION

LOCAL_ADDITIONAL_DEPENDENCIES :=
LOCAL_SRC_FILES :=
