/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <asm-generic/gpio.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/pinctrl/consumer.h>
#endif
#endif

#include "lcm_drv.h"

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output);
static unsigned int LCD_RST_PIN;
static unsigned int LCD_PWR_EN;
static struct pinctrl *lcd_pinctrl1;
static struct pinctrl_state *lcd_disp_pwm;
static struct pinctrl_state *lcd_disp_pwm_gpio;

static void lcm_request_gpio_control(struct device *dev)
{
	int ret;

	LCD_RST_PIN = of_get_named_gpio(dev->of_node, "gpio_lcd_rst", 0);
	gpio_request(LCD_RST_PIN, "LCD_RST_PIN");

	LCD_PWR_EN = of_get_named_gpio(dev->of_node, "lcd_power_gpio", 0);
	gpio_request(LCD_PWR_EN, "LCD_PWR_EN");

	lcd_pinctrl1 = devm_pinctrl_get(dev);
	if (IS_ERR(lcd_pinctrl1)) {
		ret = PTR_ERR(lcd_pinctrl1);
		pr_err("Cannot find lcd_pinctrl1 %d!\n", ret);
	}

	lcd_disp_pwm = pinctrl_lookup_state(lcd_pinctrl1, "disp_pwm");
	if (IS_ERR(lcd_pinctrl1)) {
		ret = PTR_ERR(lcd_pinctrl1);
		pr_err("Cannot find lcd_disp_pwm %d!\n", ret);
	}

	lcd_disp_pwm_gpio = pinctrl_lookup_state(lcd_pinctrl1, "disp_pwm_gpio");
	if (IS_ERR(lcd_pinctrl1)) {
		ret = PTR_ERR(lcd_pinctrl1);
		pr_err("Cannot find lcd_disp_pwm_gpio %d!\n", ret);
	}
}

static int lcm_driver_probe(struct device *dev, void const *data)
{
	lcm_request_gpio_control(dev);
	return 0;
}

static const struct of_device_id lcm_platform_of_match[] = {
    {
        .compatible = "er88577b,er88577b_dsi_vdo",
        .data = 0,
    }, {
         /* sentinel */
    }
};

MODULE_DEVICE_TABLE(of, platform_of_match);

static int lcm_platform_probe(struct platform_device *pdev)
{
    const struct of_device_id *id;
    id = of_match_node(lcm_platform_of_match, pdev->dev.of_node);
    if (!id){
        return -ENODEV;
        pr_err("LCM: lcm_platform_probe failed\n");
    }
    return lcm_driver_probe(&pdev->dev, id->data);
}

static struct platform_driver lcm_driver = {
    .probe = lcm_platform_probe,
    .driver = {
        .name = "er88577b_dsi_vdo",
        .owner = THIS_MODULE,
        .of_match_table = lcm_platform_of_match,
    },
};

static int __init lcm_init(void)
{
    if (platform_driver_register(&lcm_driver)) {
        pr_err("LCM: failed to register this driver!\n");
        return -ENODEV;
    }
    return 0;
}
static void __exit lcm_exit(void)
{
    platform_driver_unregister(&lcm_driver);
}

late_initcall(lcm_init);
module_exit(lcm_exit);
MODULE_AUTHOR("mediatek");
MODULE_DESCRIPTION("LCM display subsystem driver");
MODULE_LICENSE("GPL");

/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define FRAME_WIDTH  (600)
#define FRAME_HEIGHT (1024)

#define GPIO_OUT_ONE  1
#define GPIO_OUT_ZERO 0

/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */
static struct LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define dsi_set_cmdq_V3(para_tbl,size,force_update)         lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                      lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                  lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                       lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define LCM_DSI_CMD_MODE	0

#define REGFLAG_DELAY	0xFC
#define REGFLAG_END_OF_TABLE	0xFD   // END OF REGISTERS MARKER

struct LCM_setting_table {
	unsigned char cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{REGFLAG_DELAY,5,{}},
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY,50,{}},
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY,120,{}}
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x11, 0, {0x00}},
	{REGFLAG_DELAY,120,{}},
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY,10,{}},
};

/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
static void dsi_send_cmdq_tinno(unsigned cmd, unsigned char count, unsigned char *para_list,
				unsigned char force_update)
{
	unsigned int item[16];
	unsigned char dsi_cmd = (unsigned char)cmd;
	unsigned char dc;
	int index = 0, length = 0;

	memset(item, 0, sizeof(item));
	if (count + 1 > 60)
		return;

	if (count == 0) {
		item[0] = 0x0500 | (dsi_cmd << 16);
		length = 1;
	} else if (count == 1) {
		item[0] = 0x1500 | (dsi_cmd << 16) | (para_list[0] << 24);
		length = 1;
	} else {
		item[0] = 0x3902 | ((count + 1) << 16);	/* Count include command. */
		++length;
		while (1) {
			if (index == count + 1)
				break;
			if (index == 0)
				dc = cmd;
			else
				dc = para_list[index - 1];
			/* an item make up of 4data. */
			item[index / 4 + 1] |= (dc << (8 * (index % 4)));
			if (index % 4 == 0)
				++length;
			++index;
		}
	}

	dsi_set_cmdq(item, length, force_update);
}
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for (i = 0; i < count; i++) {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;

            default:
            //dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
            dsi_send_cmdq_tinno(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}


static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
#ifdef BUILD_LK
	mt_set_gpio_mode(GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO, output);
#else
	gpio_direction_output(GPIO, output);
	gpio_set_value(GPIO, output);
#endif
}

static void lcm_init_power(void)
{
	pr_err("[Kernel/LCM] lcm_init_power() enter\n");
	lcm_set_gpio_output(LCD_PWR_EN, GPIO_OUT_ONE);
}

static void lcm_suspend_power(void)
{
	MDELAY(5);
	lcm_set_gpio_output(LCD_RST_PIN, GPIO_OUT_ZERO);
	MDELAY(5);
	lcm_set_gpio_output(LCD_PWR_EN, GPIO_OUT_ZERO);
	MDELAY(5);
}

static void lcm_resume_power(void)
{
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */
static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dsi.mode = BURST_VDO_MODE;// BURST_VDO_MODE;

	// DSI
	params->dsi.LANE_NUM = LCM_THREE_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 2;
	params->dsi.vertical_frontporch = 14;
	params->dsi.vertical_active_line = FRAME_HEIGHT;//hight

	params->dsi.horizontal_sync_active = 6;
	params->dsi.horizontal_backporch = 16;
	params->dsi.horizontal_frontporch = 16;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;//=wight

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.pll_select = 0;	//0: MIPI_PLL; 1: LVDS_PLL
	params->dsi.PLL_CLOCK = 170;//168//this value must be in MTK suggested table 182
	/*Modify esd mode for AX3207-999 by huixiaolong at 2021/05/27 start*/
	params->dsi.cont_clock = 0;//if not config this para, must config other 7 or 3 paras to gen. PLL
        params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.HS_TRAIL = 4;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x09;
	params->dsi.lcm_esd_check_table[0].count = 4;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
	params->dsi.lcm_esd_check_table[0].para_list[1] = 0x73;
	params->dsi.lcm_esd_check_table[0].para_list[2] = 0x04;
	params->dsi.lcm_esd_check_table[0].para_list[3] = 0x40;
	/*Modify esd mode for AX3207-999 by huixiaolong at 2021/05/27 end*/
}

static void lcm_init_lcm(void)
{
	printk("[Kernel/LCM] ---lcm_init--er88577b-\n");
	MDELAY(10);
	lcm_set_gpio_output(LCD_RST_PIN, GPIO_OUT_ONE);
	MDELAY(10);
	lcm_set_gpio_output(LCD_RST_PIN, GPIO_OUT_ZERO);
	MDELAY(10);
	lcm_set_gpio_output(LCD_RST_PIN, GPIO_OUT_ONE);
	MDELAY(120);

	pinctrl_select_state(lcd_pinctrl1, lcd_disp_pwm);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	printk("[Kernel/LCM] lcm_suspend() enter\n");
	push_table(lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);

	pinctrl_select_state(lcd_pinctrl1, lcd_disp_pwm_gpio);
}

static void lcm_resume(void)
{
	printk("[Kernel/LCM] lcm_resume() enter\n");
	lcm_init_lcm();
}

static unsigned int lcm_compare_id(void)
{
	printk("[LCM][%s]------er88577b_600hd_dsi_jc",__func__);
	return 1;
}

struct LCM_DRIVER er88577b_600hd_dsi_vdo_jc_lcm_drv = {
	.name = "er88577b_600hd_dsi_vdo_jc",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init_lcm,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
};
