// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Author: Michael Hsiao <michael.hsiao@mediatek.com>
 */

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   AudDrv_Gpio.c
 *
 * Project:
 * --------
 *   MT6797  Audio Driver GPIO
 *
 * Description:
 * ------------
 *   Audio register
 *
 * Author:
 * -------
 * George
 *
 *------------------------------------------------------------------------------
 *
 *
 ******************************************************************************
 */

/*****************************************************************************
 *                     C O M P I L E R   F L A G S
 *****************************************************************************/

/*****************************************************************************
 *                E X T E R N A L   R E F E R E N C E S
 *****************************************************************************/
#include "mtk-auddrv-gpio.h"
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>

#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_fdt.h>

struct pinctrl *pinctrlaud;

#define MT6755_PIN 1

enum audio_system_gpio_type {
	GPIO_AUD_CLK_MOSI_OFF,
	GPIO_AUD_CLK_MOSI_ON,
	GPIO_AUD_DAT_MISO_OFF,
	GPIO_AUD_DAT_MISO_ON,
	GPIO_AUD_DAT_MOSI_OFF,
	GPIO_AUD_DAT_MOSI_ON,
	GPIO_SMARTPA_ON,
	GPIO_SMARTPA_OFF,
#if MT6755_PIN
	/* GPIO_DEFAULT, */
	GPIO_EXTAMP_HIGH,
	GPIO_EXTAMP_LOW,
#endif
	GPIO_NUM
};

struct audio_gpio_attr {
	const char *name;
	bool gpio_prepare;
	struct pinctrl_state *gpioctrl;
};

static struct audio_gpio_attr aud_gpios[GPIO_NUM] = {
		[GPIO_AUD_CLK_MOSI_OFF] = {"aud_clk_mosi_off", false, NULL},
		[GPIO_AUD_CLK_MOSI_ON] = {"aud_clk_mosi_on", false, NULL},
		[GPIO_AUD_DAT_MISO_OFF] = {"aud_dat_miso_off", false, NULL},
		[GPIO_AUD_DAT_MISO_ON] = {"aud_dat_miso_on", false, NULL},
		[GPIO_AUD_DAT_MOSI_OFF] = {"aud_dat_mosi_off", false, NULL},
		[GPIO_AUD_DAT_MOSI_ON] = {"aud_dat_mosi_on", false, NULL},

		[GPIO_SMARTPA_ON] = {"aud_smartpa_on", false, NULL},
		[GPIO_SMARTPA_OFF] = {"aud_smartpa_off", false, NULL},

#if MT6755_PIN
		/* [GPIO_DEFAULT] = {"default", false, NULL}, */
		[GPIO_EXTAMP_HIGH] = {"extamp-pullhigh", false, NULL},
		[GPIO_EXTAMP_LOW] = {"extamp-pulllow", false, NULL},
#endif
};

static DEFINE_MUTEX(gpio_request_mutex);

void AudDrv_GPIO_probe(void *dev)
{
	int ret;
	int i = 0;

	pr_debug("%s\n", __func__);

	pinctrlaud = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrlaud)) {
		ret = PTR_ERR(pinctrlaud);
		pr_err("Cannot find pinctrlaud!\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(aud_gpios); i++) {
		aud_gpios[i].gpioctrl =
			pinctrl_lookup_state(pinctrlaud, aud_gpios[i].name);
		if (IS_ERR(aud_gpios[i].gpioctrl)) {
			ret = PTR_ERR(aud_gpios[i].gpioctrl);
			pr_err("%s pinctrl_lookup_state %s fail %d\n", __func__,
			       aud_gpios[i].name, ret);
		} else {
			aud_gpios[i].gpio_prepare = true;
		}
	}
}

static int AudDrv_GPIO_Select(enum audio_system_gpio_type _type)
{
#ifndef CONFIG_FPGA_EARLY_PORTING
	int ret = 0;

	if (_type < 0 || _type >= GPIO_NUM) {
		pr_err("%s(), error, invaild gpio type %d\n", __func__, _type);
		return -EINVAL;
	}

	if (!aud_gpios[_type].gpio_prepare) {
		pr_err("%s(), error, gpio type %d not prepared\n", __func__,
		       _type);
		return -EIO;
	}

	ret = pinctrl_select_state(pinctrlaud, aud_gpios[_type].gpioctrl);
	if (ret) {
		pr_err("%s(), error, can not set gpio type %d\n", __func__,
		       _type);
	}
	return ret;
#else
	return 0;
#endif
}

static bool AudDrv_GPIO_IsValid(enum audio_system_gpio_type _type)
{
#ifndef CONFIG_FPGA_EARLY_PORTING
	if (_type < 0 || _type >= GPIO_NUM)
		return false;

	if (!aud_gpios[_type].gpio_prepare)
		return false;

	return true;
#else
	return true;
#endif
}


static int set_aud_clk_mosi(bool _enable)
{
/*
 * scp also need this gpio on mt6797,
 * don't switch gpio if they exist.
 */
#ifndef CONFIG_MTK_TINYSYS_SCP_SUPPORT
	static int aud_clk_mosi_counter;

	if (_enable) {
		aud_clk_mosi_counter++;
		if (aud_clk_mosi_counter == 1)
			return AudDrv_GPIO_Select(GPIO_AUD_CLK_MOSI_ON);
	} else {
		if (aud_clk_mosi_counter > 0) {
			aud_clk_mosi_counter--;
		} else {
			aud_clk_mosi_counter = 0;
			pr_info("%s(), counter %d <= 0\n", __func__,
				aud_clk_mosi_counter);
		}

		if (aud_clk_mosi_counter == 0)
			return AudDrv_GPIO_Select(GPIO_AUD_CLK_MOSI_OFF);
	}
#endif
	return 0;
}

static int set_aud_dat_mosi(bool _enable)
{
	if (_enable)
		return AudDrv_GPIO_Select(GPIO_AUD_DAT_MOSI_ON);
	else
		return AudDrv_GPIO_Select(GPIO_AUD_DAT_MOSI_OFF);
}

static int set_aud_dat_miso(bool _enable, enum soc_aud_digital_block _usage)
{
	static bool adda_enable;

	switch (_usage) {
	case Soc_Aud_Digital_Block_ADDA_UL:
		adda_enable = _enable;
		break;
	default:
		return -EINVAL;
	}

	if (adda_enable)
		return AudDrv_GPIO_Select(GPIO_AUD_DAT_MISO_ON);
	else
		return AudDrv_GPIO_Select(GPIO_AUD_DAT_MISO_OFF);
}

int AudDrv_GPIO_Request(bool _enable, enum soc_aud_digital_block _usage)
{
	mutex_lock(&gpio_request_mutex);
	switch (_usage) {
	case Soc_Aud_Digital_Block_ADDA_DL:
		set_aud_clk_mosi(_enable);
		set_aud_dat_mosi(_enable);
		break;
	case Soc_Aud_Digital_Block_ADDA_UL:
		set_aud_clk_mosi(_enable);
		set_aud_dat_miso(_enable, _usage);
		break;
	case Soc_Aud_Digital_Block_ADDA_UL2:
		set_aud_clk_mosi(_enable);
		break;
	case Soc_Aud_Digital_Block_ADDA_VOW:
		set_aud_dat_miso(_enable, _usage);
		break;
	case Soc_Aud_Digital_Block_ADDA_ANC:
		set_aud_clk_mosi(_enable);
		break;
	case Soc_Aud_Digital_Block_ADDA_ALL:
		set_aud_clk_mosi(_enable);
		set_aud_dat_mosi(_enable);
		set_aud_dat_miso(_enable, Soc_Aud_Digital_Block_ADDA_UL);
		break;
	default:
		mutex_unlock(&gpio_request_mutex);
		return -EINVAL;
	}
	mutex_unlock(&gpio_request_mutex);
	return 0;
}

int AudDrv_GPIO_SMARTPA_Select(int mode)
{
	int retval = 0;

	mutex_lock(&gpio_request_mutex);
	switch (mode) {
	case 0:
		if (AudDrv_GPIO_IsValid(GPIO_SMARTPA_OFF))
			retval = AudDrv_GPIO_Select(GPIO_SMARTPA_OFF);
		break;
	case 1:
		if (AudDrv_GPIO_IsValid(GPIO_SMARTPA_ON))
			retval = AudDrv_GPIO_Select(GPIO_SMARTPA_ON);
		break;
	default:
		pr_err("%s(), invalid mode = %d", __func__, mode);
		retval = -1;
	}
	mutex_unlock(&gpio_request_mutex);
	return retval;
}

int AudDrv_GPIO_EXTAMP_Select(int bEnable, int mode)
{
	int retval = 0;

#if MT6755_PIN
	int extamp_mode;
	int i;

	mutex_lock(&gpio_request_mutex);
	if (bEnable == 1) {
		if (mode == 1)
			extamp_mode = 1;
		else if (mode == 2)
			extamp_mode = 2;
		else if (mode == 10)
			extamp_mode = 10;
		else
			extamp_mode = 3; /* default mode is 3 */

		if (aud_gpios[GPIO_EXTAMP_HIGH].gpio_prepare) {
			for (i = 0; i < extamp_mode; i++) {
				retval = pinctrl_select_state(
					pinctrlaud,
					aud_gpios[GPIO_EXTAMP_LOW].gpioctrl);
				if (retval)
					pr_info("could not set aud_gpios[GPIO_EXTAMP_LOW] pins\n");
				udelay(2);
				retval = pinctrl_select_state(
					pinctrlaud,
					aud_gpios[GPIO_EXTAMP_HIGH].gpioctrl);
				if (retval)
					pr_info("could not set aud_gpios[GPIO_EXTAMP_HIGH] pins\n");
				udelay(2);
			}
		}
	} else {
		if (aud_gpios[GPIO_EXTAMP_LOW].gpio_prepare) {
			retval = pinctrl_select_state(
				pinctrlaud,
				aud_gpios[GPIO_EXTAMP_LOW].gpioctrl);
			if (retval)
				pr_info("could not set aud_gpios[GPIO_EXTAMP_LOW] pins\n");
		}
	}
	mutex_unlock(&gpio_request_mutex);
#endif
	return retval;
}
