ifdef MTK_DTBOIMAGE_DTS
#ifdef MTK_DTBOIMAGE_DWS

ifneq (true,$(strip $(TARGET_NO_KERNEL)))
ifneq ($(LINUX_KERNEL_VERSION),)

ifeq (,$(KERNEL_OUT))
include $(LINUX_KERNEL_VERSION)/kenv.mk
endif

DRVGEN_TOOL := prebuilts/build/DrvGen.py
DRVGEN_FIG := $(wildcard $(dir $(DRVGEN_TOOL))config/*.fig)
define drvgen_dws_to_dtsi
kernel: $(2)/cust.dtsi
$(2)/cust.dtsi: drvgen_tool := $(DRVGEN_TOOL)
$(2)/cust.dtsi: prj_path := $(2)
$(2)/cust.dtsi: $(DRVGEN_TOOL) $(1) $(DRVGEN_FIG)
	$(hide) mkdir -p $$(dir $$@)
	python $$(drvgen_tool) $(1) $$(prj_path) $$(prj_path) cust_dtsi
endef

MTK_DTBOIMAGE_DTB :=
MTK_DTBOIMAGE_DTSI :=
$(foreach i,$(MTK_DTBOIMAGE_DTS),\
  $(eval base_prj := $(shell grep -m 1 '#include [<\"].*\/cust\.dtsi[>\"]' $(i) | sed 's/#include [<"]//g' | sed 's/\/cust\.dtsi[>"]//g' | sed 's/\/\*//g' | sed 's/\*\///g' | sed 's/ //g'))\
  $(if $(strip $(base_prj)),\
    $(eval prj_path := $(KERNEL_OUT)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/$(base_prj))\
    $(eval dws_path := $(KERNEL_DIR)/drivers/misc/mediatek/dws/$(MTK_PLATFORM_DIR)/$(base_prj).dws)\
    $(if $(filter $(prj_path),$(MTK_DTBOIMAGE_DTSI)),,\
      $(eval $(call drvgen_dws_to_dtsi,$(dws_path),$(prj_path)))\
      $(eval MTK_DTBOIMAGE_DTSI += $(prj_path))\
      $(eval LOCAL_ADDITIONAL_DEPENDENCIES := $(prj_path)/cust.dtsi)\
    )\
  )\
  $(eval LOCAL_ADDITIONAL_DEPENDENCIES_D := $(MTK_DTBIMAGE_DTB))\
  $(eval LOCAL_ADDITIONAL_DEPENDENCIES_D += $(MTK_DTBOIMAGE_DTB))\
  $(eval LOCAL_SRC_FILES := $(i))\
  $(eval include prebuilts/build/build_dtb.mk)\
  $(eval MTK_DTBOIMAGE_DTB += $(my_kernel_dtb))\
)

kernel: $(my_kernel_dtb).merge
my_dtbo_id := 0
define mk_dtboimg_cfg
echo $(1) >>$(2);\
echo " id=$(my_dtbo_id)" >>$(2);\
$(eval my_dtbo_id := $(call int_plus,$(my_dtbo_id),1))
endef

MTK_DTBOIMAGE_CFG := $(KERNEL_OUT)/arch/$(KERNEL_TARGET_ARCH)/boot/dts/dtboimg.cfg
$(MTK_DTBOIMAGE_CFG): PRIVATE_DTB := $(MTK_DTBOIMAGE_DTB)
$(MTK_DTBOIMAGE_CFG): $(MTK_DTBOIMAGE_DTB)
	$(hide) rm -f $@.tmp
	$(hide) $(foreach f,$(PRIVATE_DTB),$(call mk_dtboimg_cfg,$(f),$@.tmp))
	$(hide) if ! cmp -s $@.tmp $@; then \
		mv $@.tmp $@; \
	else \
		rm $@.tmp; \
	fi

endif#TARGET_NO_KERNEL
endif#LINUX_KERNEL_VERSION

#endif#MTK_DTBOIMAGE_DWS
endif#MTK_DTBOIMAGE_DTS
