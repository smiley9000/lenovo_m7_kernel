#
# define some marco
#
define my-dir
$(strip \
	  $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
	    $(if $(filter $(BUILD_SYSTEM)/% $(OUT_DIR)/%,$(LOCAL_MODULE_MAKEFILE)), \
		    $(error my-dir must be called before including any other makefile.) \
			   , \
			       $(patsubst %/,%,$(dir $(LOCAL_MODULE_MAKEFILE))) \
				      ) \
					   )
endef

define copy-file-to-target
@mkdir -p $(dir $@)
$(hide) rm -f $@
$(hide) cp "$<" "$@"
endef

define copy-file-to-new-target
@mkdir -p $(dir $@)
$(hide) rm -f $@
$(hide) cp $< $@
endef
