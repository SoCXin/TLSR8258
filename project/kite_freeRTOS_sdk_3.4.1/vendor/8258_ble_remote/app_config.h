/********************************************************************************************************
 * @file     app_confih.h
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif




/////////////////// FEATURE SELECT /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE				1
#define PM_DEEPSLEEP_RETENTION_ENABLE		1
#define BLE_REMOTE_SECURITY_ENABLE      	1
#define BLE_REMOTE_OTA_ENABLE				1
#define REMOTE_IR_ENABLE					0
#define BATT_CHECK_ENABLE       			0   //must enable
#define BLE_AUDIO_ENABLE					0
#define BLT_APP_LED_ENABLE					0
#define BLT_TEST_SOFT_TIMER_ENABLE			0

#define BLE_IOS_ANCS_ENABLE					1

//firmware check
#define FIRMWARES_SIGNATURE_ENABLE          0


/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                   DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#define	LOW_BATT_FLG					    BIT(0)
#define CONN_DEEP_FLG	                    BIT(1) //if 1: conn deep, 0: adv deep
#define IR_MODE_DEEP_FLG	 				BIT(2) //if 1: IR mode, 0: BLE mode

#define	OS_ENABLE							1
#define OS_SLEEP_ON_IDLE_TASK				(OS_ENABLE && 1)
#define OS_APP_WAKEUP_ENABLE				(OS_ENABLE)


#if (BATT_CHECK_ENABLE)
//telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	//use PB7 output high level, then adc measure this high level voltage
	#define GPIO_VBAT_DETECT				GPIO_PB7
	#define PB7_FUNC						AS_GPIO
	#define PB7_INPUT_ENABLE				0
	#define ADC_INPUT_PCHN					B7P    //corresponding  ADC_InputPchTypeDef in adc.h
#endif





/////////////////// Clock  /////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  	16000000

enum{
	CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
};



/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		500  //m#define DEBUG_GPIO_ENABLE			0s





///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
	ATT_H_START = 0,


	//// Gap ////
	/**********************************************************************************************/
	GenericAccess_PS_H, 					//UUID: 2800, 	VALUE: uuid 1800
	GenericAccess_DeviceName_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	GenericAccess_DeviceName_DP_H,			//UUID: 2A00,   VALUE: device name
	GenericAccess_Appearance_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	GenericAccess_Appearance_DP_H,			//UUID: 2A01,	VALUE: appearance
	CONN_PARAM_CD_H,						//UUID: 2803, 	VALUE:  			Prop: Read
	CONN_PARAM_DP_H,						//UUID: 2A04,   VALUE: connParameter


	//// gatt ////
	/**********************************************************************************************/
	GenericAttribute_PS_H,					//UUID: 2800, 	VALUE: uuid 1801
	GenericAttribute_ServiceChanged_CD_H,	//UUID: 2803, 	VALUE:  			Prop: Indicate
	GenericAttribute_ServiceChanged_DP_H,   //UUID:	2A05,	VALUE: service change
	GenericAttribute_ServiceChanged_CCB_H,	//UUID: 2902,	VALUE: serviceChangeCCC


	//// device information ////
	/**********************************************************************************************/
	DeviceInformation_PS_H,					//UUID: 2800, 	VALUE: uuid 180A
	DeviceInformation_pnpID_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	DeviceInformation_pnpID_DP_H,			//UUID: 2A50,	VALUE: PnPtrs


	//// HID ////
	/**********************************************************************************************/
	HID_PS_H, 								//UUID: 2800, 	VALUE: uuid 1812

	//include
	HID_INCLUDE_H,							//UUID: 2802, 	VALUE: include

	//protocol
	HID_PROTOCOL_MODE_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	HID_PROTOCOL_MODE_DP_H,					//UUID: 2A4E,	VALUE: protocolMode

	//boot keyboard input report
	HID_BOOT_KB_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_BOOT_KB_REPORT_INPUT_DP_H,			//UUID: 2A22, 	VALUE: bootKeyInReport
	HID_BOOT_KB_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: bootKeyInReportCCC

	//boot keyboard output report
	HID_BOOT_KB_REPORT_OUTPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_BOOT_KB_REPORT_OUTPUT_DP_H,		    //UUID: 2A32, 	VALUE: bootKeyOutReport

	//consume report in
	HID_CONSUME_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_CONSUME_REPORT_INPUT_DP_H,			//UUID: 2A4D, 	VALUE: reportConsumerIn
	HID_CONSUME_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: reportConsumerInCCC
	HID_CONSUME_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_CONSUMER, TYPE_INPUT

	//keyboard report in
	HID_NORMAL_KB_REPORT_INPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_NORMAL_KB_REPORT_INPUT_DP_H,		//UUID: 2A4D, 	VALUE: reportKeyIn
	HID_NORMAL_KB_REPORT_INPUT_CCB_H,		//UUID: 2902, 	VALUE: reportKeyInInCCC
	HID_NORMAL_KB_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

	//keyboard report out
	HID_NORMAL_KB_REPORT_OUTPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_NORMAL_KB_REPORT_OUTPUT_DP_H,  		//UUID: 2A4D, 	VALUE: reportKeyOut
	HID_NORMAL_KB_REPORT_OUTPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT

	// report map
	HID_REPORT_MAP_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read
	HID_REPORT_MAP_DP_H,					//UUID: 2A4B, 	VALUE: reportKeyIn
	HID_REPORT_MAP_EXT_REF_H,				//UUID: 2907 	VALUE: extService

	//hid information
	HID_INFORMATION_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read
	HID_INFORMATION_DP_H,					//UUID: 2A4A 	VALUE: hidInformation

	//control point
	HID_CONTROL_POINT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: write_without_rsp
	HID_CONTROL_POINT_DP_H,					//UUID: 2A4C 	VALUE: controlPoint


	//// battery service ////
	/**********************************************************************************************/
	BATT_PS_H, 								//UUID: 2800, 	VALUE: uuid 180f
	BATT_LEVEL_INPUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	BATT_LEVEL_INPUT_DP_H,					//UUID: 2A19 	VALUE: batVal
	BATT_LEVEL_INPUT_CCB_H,					//UUID: 2902, 	VALUE: batValCCC


	//// Ota ////
	/**********************************************************************************************/
	OTA_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	OTA_CMD_OUT_CD_H,						//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	OTA_CMD_OUT_DP_H,						//UUID: telink ota uuid,  VALUE: otaData
	OTA_CMD_OUT_DESC_H,						//UUID: 2901, 	VALUE: otaName



	ATT_END_H,

}ATT_HANDLE;



#define DEBUG_GPIO_ENABLE					0

#define GPIO_CHN0							GPIO_PC0  // deep retention
#define GPIO_CHN1							GPIO_PC1  // proto task
#define GPIO_CHN2							GPIO_PC4  // Task Switching
#define GPIO_CHN3							GPIO_PC5  // PC2/PC3 may used for external crystal input
#define GPIO_CHN4							GPIO_PC6  // PC2/PC3 may used for external crystal input
#define GPIO_CHN5							GPIO_PC7  // BRX
#define GPIO_CHN6							GPIO_PB4  // RX
#define GPIO_CHN7							GPIO_PB5  // Interrupt
#define GPIO_CHN8							GPIO_PD5  // Irq disable enable
#if(DEBUG_GPIO_ENABLE)
	//define debug GPIO here according to your hardware
	#define PC0_OUTPUT_ENABLE					1
	#define PC1_OUTPUT_ENABLE					1
	#define PC4_OUTPUT_ENABLE					1
	#define PC5_OUTPUT_ENABLE					1
	#define PC6_OUTPUT_ENABLE					1
	#define PC7_OUTPUT_ENABLE					1
	#define PB4_OUTPUT_ENABLE					1
	#define PB5_OUTPUT_ENABLE					1
	#define PD5_OUTPUT_ENABLE					1

#endif  //end of DEBUG_GPIO_ENABLE

#define DEBUG_GPIO(gpio, en)	do{if(DEBUG_GPIO_ENABLE) gpio_write(gpio, en);}while(0)


#define BLE_PHYTEST_MODE						PHYTEST_MODE_DISABLE


#define GPIO_WAKEUP_MCU						GPIO_PD0   //module wakeup mcu
#define	PD0_FUNC							AS_GPIO
#define PD0_INPUT_ENABLE					0
#define	PD0_OUTPUT_ENABLE					1
#define	PD0_DATA_OUT						0
#define GPIO_WAKEUP_MCU_HIGH				do{gpio_set_output_en(GPIO_WAKEUP_MCU, 1); gpio_write(GPIO_WAKEUP_MCU, 1);}while(0)
#define GPIO_WAKEUP_MCU_LOW					do{gpio_set_output_en(GPIO_WAKEUP_MCU, 1); gpio_write(GPIO_WAKEUP_MCU, 0);}while(0)
#define GPIO_WAKEUP_MCU_FLOAT				do{gpio_set_output_en(GPIO_WAKEUP_MCU, 0); gpio_write(GPIO_WAKEUP_MCU, 0);}while(0)

#define UART_PRINT_DEBUG_ENABLE							0

#if (UART_PRINT_DEBUG_ENABLE)
#define USB_PRINT_DEBUG_ENABLE							1
//1M baud rate,should Not bigger than 1M, when system clock is 16M.
#define PRINT_BAUD_RATE             					115200///1000000
#define DEBUG_INFO_TX_PIN           					GPIO_PB1
#define PULL_WAKEUP_SRC_PB4        					PM_PIN_PULLUP_1M
#define PB4_OUTPUT_ENABLE								1
#define PB4_INPUT_ENABLE								0
#endif
#include "application/print/u_printf.h"
#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
