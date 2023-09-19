// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 */
/* Liverpool code for AX3207-366 by liruiju at 20-11-05 start */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/iio/consumer.h>
#include <linux/iio/iio.h>
#include <linux/platform_device.h>	/* platform device */

static struct platform_device *md_adc_pdev;
static int adc_num;
static int adc_val;

static int ccci_get_thubadc_info(struct device *dev)
{
	int ret, val;
	struct iio_channel *md_channel;

	md_channel = iio_channel_get(dev, "md-thubchannel");

	ret = IS_ERR(md_channel);
	if (ret) {
		if (PTR_ERR(md_channel) == -EPROBE_DEFER) {
			pr_err("%s EPROBE_DEFER\r\n",
					__func__);
			return -EPROBE_DEFER;
		}
		pr_err("fail to get iio channel (%d)", ret);
		goto Fail;
	}

	adc_num = md_channel->channel->channel;
	ret = iio_read_channel_raw(md_channel, &val);
	iio_channel_release(md_channel);
	if (ret < 0) {
		pr_err("iio_read_channel_raw fail");
		goto Fail;
	}

	adc_val = val;
	pr_err("[%s]:md_ch = %d, val = %d\n", 
    __func__, adc_num, adc_val);
	return ret;
Fail:
	return -1;

}

/*int ccci_get_adc_num(void)
{
	return adc_num;
}
EXPORT_SYMBOL(ccci_get_adc_num);

int ccci_get_adc_val(void)
{
	return adc_val;
}
EXPORT_SYMBOL(ccci_get_adc_val);*/

signed int t_hub_get_bat_voltage(void)
{
	struct iio_channel *channel;
	int ret, val, number;

	if (!md_adc_pdev)
		return -1;
	channel = iio_channel_get(&md_adc_pdev->dev, "md-thubchannel");

	ret = IS_ERR(channel);
	if (ret) {
		pr_err("fail to get iio channel 4 (%d)", ret);
		goto BAT_Fail;
	}
	number = channel->channel->channel;
	ret = iio_read_channel_processed(channel, &val);
	iio_channel_release(channel);
	if (ret < 0) {
		pr_err("iio_read_channel_processed fail");
		goto BAT_Fail;
	}
	pr_err("md_battery = %d, val = %d", number, val);

	return val;
BAT_Fail:
	return -1;
}
EXPORT_SYMBOL(t_hub_get_bat_voltage);

int get_thubadc_probe(struct platform_device *pdev)
{
	int ret;

	ret = ccci_get_thubadc_info(&pdev->dev);
	if (ret < 0) {
		pr_err("ccci get thub adc info fail");
		return ret;
	}
	md_adc_pdev = pdev;
	return 0;
}


static const struct of_device_id ccci_thubadc_of_ids[] = {
	{.compatible = "mediatek,md_thubadc"},
	{}
};


static struct platform_driver ccci_thubadc_driver = {

	.driver = {
			.name = "ccci_thubadc",
			.of_match_table = ccci_thubadc_of_ids,
	},

	.probe = get_thubadc_probe,
};

static int __init ccci_thubadc_init(void)
{
	int ret;

	ret = platform_driver_register(&ccci_thubadc_driver);
	if (ret) {
		pr_err("ccci auxadc driver init fail %d", ret);
		return ret;
	}
	return 0;
}

module_init(ccci_thubadc_init);

MODULE_AUTHOR("zh");
MODULE_DESCRIPTION("ccci thubadc driver");
MODULE_LICENSE("GPL");
/* Liverpool code for AX3207-366 by liruiju at 20-11-05 end */