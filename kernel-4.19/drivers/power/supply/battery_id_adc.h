/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020.
 */
/* AX3207 code for AX3207-334 by zhanghuan 2020-11-6 start */
#ifndef __BATTERY_ID_ADC__
#define __BATTERY_ID_ADC__

#ifdef CONFIG_MEDIATEK_MT6577_AUXADC
int bat_id_get_adc_num(void);
int bat_id_get_adc_val(void);
#endif

signed int battery_get_bat_id_voltage(void);

#endif
/* AX3207 code for AX3207-334 by zhanghuan 2020-11-6 end */
