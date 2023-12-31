/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 hi556mipi_Sensor.h
 *
 * Project:
 * --------
 *	 ALPS
 *
 * Description:
 * ------------
 *	 CMOS sensor header file
 *
 ****************************************************************************/
#ifndef _HI556MIPI_SENSOR_H
#define _HI556MIPI_SENSOR_H

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

enum{
	IMGSENSOR_MODE_INIT,
	IMGSENSOR_MODE_PREVIEW,
	IMGSENSOR_MODE_CAPTURE,
	IMGSENSOR_MODE_VIDEO,
	IMGSENSOR_MODE_HIGH_SPEED_VIDEO,
	IMGSENSOR_MODE_SLIM_VIDEO,
	IMGSENSOR_MODE_CUSTOM1,
	IMGSENSOR_MODE_CUSTOM2,
	IMGSENSOR_MODE_CUSTOM3,
	IMGSENSOR_MODE_CUSTOM4,
	IMGSENSOR_MODE_CUSTOM5,
} IMGSENSOR_MODE;

struct hi556_otp_struct 
{
	int Base_Info_Flag;	//bit[7]:info, bit[6]:wb
	int module_integrator_id;
	int AF_Flag;
	int production_year;
	int production_month;
	int production_day;
	int awb_lsc_station;
	int sensor_id;
	int lens_id;
	int  module_version;
	int  software_version;
	//int vcm_id;
	//int Driver_ic_id;
	//int F_num_id;
	int WB_FLAG;
	int LSC_FLAG;
	int wb_data[30];
	int lsc_data[1900];
	int AF_FLAG;
	int af_data[5];
	int infocheck;
	int wbcheck;
	int checksum;
};

struct imgsensor_mode_struct {
	kal_uint32 pclk;
	kal_uint32 linelength;
	kal_uint32 framelength;

	kal_uint8 startx;
	kal_uint8 starty;

	kal_uint16 grabwindow_width;
	kal_uint16 grabwindow_height;
	kal_uint32 mipi_pixel_rate;
	kal_uint8 mipi_data_lp2hs_settle_dc;

	/*	 following for GetDefaultFramerateByScenario()	*/
	kal_uint16 max_framerate;

} imgsensor_mode_struct;

/* SENSOR PRIVATE STRUCT FOR VARIABLES*/
struct imgsensor_struct {
	kal_uint8 mirror;				//mirrorflip information

	kal_uint8 sensor_mode; //record IMGSENSOR_MODE enum value

	kal_uint32 shutter;				//current shutter
	kal_uint16 gain;				//current gain

	kal_uint32 pclk;				//current pclk

	kal_uint32 frame_length;		//current framelength
	kal_uint32 line_length;			//current linelength

	kal_uint32 min_frame_length; //current min framelength to max
	kal_int32 dummy_pixel;			//current dummypixel
	kal_int32 dummy_line;			//current dummline

	kal_uint16 current_fps;			//current max fps
	kal_bool   autoflicker_en; //record autoflicker enable or disable
	kal_bool test_pattern; //record test pattern mode or not
	enum MSDK_SCENARIO_ID_ENUM current_scenario_id; //current scenario
	kal_bool  ihdr_en;				//ihdr enable or disable

	kal_uint8 i2c_write_id; //record current sensor's i2c write id
} imgsensor_struct;

/* SENSOR PRIVATE STRUCT FOR CONSTANT*/
struct imgsensor_info_struct {
	kal_uint16 sensor_id; //record sensor id defined in Kd_imgsensor.h
	kal_uint32 checksum_value; //checksum value for Camera Auto Test
	struct imgsensor_mode_struct pre; //preview scenario
	struct imgsensor_mode_struct cap; //capture scenario
	struct imgsensor_mode_struct cap1; //capture for PIP 24fps
	struct imgsensor_mode_struct cap2; //capture for PIP 15ps
	struct imgsensor_mode_struct normal_video;//normal video info
	struct imgsensor_mode_struct hs_video; //high speed video relative info
	struct imgsensor_mode_struct slim_video; //slim video for VT
	struct imgsensor_mode_struct custom1; //custom1 scenario relative info
	struct imgsensor_mode_struct custom2; //custom2 scenario relative info
	struct imgsensor_mode_struct custom3; //custom3 scenario relative info
	struct imgsensor_mode_struct custom4; //custom4 scenario relative info
	struct imgsensor_mode_struct custom5; //custom5 scenario relative info

	kal_uint8  ae_shut_delay_frame; //shutter delay frame for AE cycle
	kal_uint8  ae_sensor_gain_delay_frame; //sensorgaindelfra for AEcycle
	kal_uint8  ae_ispGain_delay_frame; //ispgaindelayframe for AEcycle
	kal_uint8  ihdr_support;		//1, support; 0,not support
	kal_uint8  ihdr_le_firstline;	//1,le first ; 0, se first
	kal_uint8  sensor_mode_num;		//support sensor mode num

	kal_uint8  cap_delay_frame;		//enter capture delay frame num
	kal_uint8  pre_delay_frame;		//enter preview delay frame num
	kal_uint8  video_delay_frame; //enter video delay frame num
	kal_uint8  hs_video_delay_frame; //enter high speed videodelayframenum
	kal_uint8  slim_video_delay_frame; //enter slim video delay frame num
	kal_uint8  custom1_delay_frame;     //enter custom1 delay frame num
	kal_uint8  custom2_delay_frame;     //enter custom1 delay frame num
	kal_uint8  custom3_delay_frame;     //enter custom1 delay frame num
	kal_uint8  custom4_delay_frame;     //enter custom1 delay frame num
	kal_uint8  custom5_delay_frame;     //enter custom1 delay frame num

	kal_uint8  margin; //sensor framelength & shutter margin
	kal_uint32 min_shutter; //min shutter
	kal_uint32 max_frame_length; //maxframelengthbysensor reg's limitation

	kal_uint8  isp_driving_current; //mclk driving current
	kal_uint8  sensor_interface_type;//sensor_interface_type
	kal_uint8  mipi_sensor_type;
	kal_uint8  mipi_settle_delay_mode;
	kal_uint8  sensor_output_dataformat;
	kal_uint8  mclk;

	kal_uint8  mipi_lane_num;		//mipi lane num
	kal_uint8  i2c_addr_table[5];
	kal_uint32  i2c_speed;     //i2c speed
} imgsensor_info_struct;


extern int iReadRegI2C(u8 *a_pSendData, u16 a_sizeSendData, u8 *a_pRecvData,
				u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData, u16 a_sizeSendData, u16 i2cId);
extern void kdSetI2CSpeed(u16 i2cSpeed);
extern int iBurstWriteReg(u8 *pData, u32 bytes, u16 i2cId);

extern int iBurstWriteReg_multi(u8 *pData, u32 bytes, u16 i2cId,
	u16 transfer_length, u16 timing);

#endif
