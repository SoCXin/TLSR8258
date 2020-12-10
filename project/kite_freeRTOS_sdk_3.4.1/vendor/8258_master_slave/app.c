/********************************************************************************************************
 * @file     app.c
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     May. 22, 2019
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

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "stack/ble/smpc/smp_ms.h"
#include "vendor/common/blt_common.h"


#include "blms_att.h"
#include "blms_pair.h"
#include "blms_host.h"
#include "blms_ota.h"


MYFIFO_INIT(blt_rxfifo, 64, 16);


int main_idle_loop (void);


const u8	tbl_advData[] = {
	 0x08, 0x09, 'M', 'S', '-', 'R', 'o', 'l', 'e',
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x03, 0x19, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
	 0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 0x08, 0x09, 'M', 'S', '-', 'R', 'o', 'l', 'e',
};





///////////////////////////////////////////
void user_init(void)
{
	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
	random_generator_init();  //this is must

	#if (APPLICATION_DONGLE)
		#if (__LOG_RT_ENABLE__)
			usb_log_init();  //trace debug
		#endif

		//set USB ID
		REG_ADDR8(0x74) = 0x62;
		REG_ADDR16(0x7e) = 0x08d0;
		REG_ADDR8(0x74) = 0x00;

		usb_dp_pullup_en (1);  //open USB enum
	#endif


///////////// BLE stack Initialization ////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);


	////// Controller Initialization  //////////
	blc_llms_initBasicMCU();
	blc_llms_initStandby_module(mac_public);						   //mandatory

	blc_llms_initAdvertising_module(); 	//adv module: 		 mandatory for BLE slave,

	blc_llms_initScanning_module(); 		//scan module: 		 mandatory for BLE master,



	blc_llms_initMultiMasterSingleSlave_module();




	rf_set_power_level_index (RF_POWER_P3p01dBm);




	////// Host Initialization  //////////
	blms_gap_init();    //gap initialization
	extern void my_att_init ();
	my_att_init (); //gatt initialization

	blc_l2cap_register_handler (app_l2cap_handler);  	//l2cap initialization



	blc_hci_registerControllerEventHandler(controller_event_callback); //controller hci event to host all processed in this func
	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE | HCI_EVT_MASK_ENCRYPTION_CHANGE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(		HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
									|	HCI_LE_EVT_MASK_ADVERTISING_REPORT \
									|   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE \
									|   HCI_LE_EVT_MASK_CONNECTION_ESTABLISH);  //connection establish: telink private event

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );

	#if (BLE_HOST_SMP_ENABLE)
		// New paring: send security_request immediately after connection complete
		// reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request.
		// TODO: modify API's name, only slave-role can use the API
		blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
		blm_host_smp_setSecurityTrigger ( MASTER_TRIGGER_SMP_FIRST_PAIRING | MASTER_TRIGGER_SMP_AUTO_CONNECT);
		blm_smp_registerSmpFinishCb(app_host_smp_finish);
		blms_smp_param_init();
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif


	extern int host_att_register_idle_func (void *p);
	host_att_register_idle_func (main_idle_loop);



	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE | HCI_EVT_MASK_ENCRYPTION_CHANGE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(		HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
									|	HCI_LE_EVT_MASK_ADVERTISING_REPORT \
									|   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE \
									|   HCI_LE_EVT_MASK_CONNECTION_ESTABLISH);  //connection establish: telink private event




	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
	bls_ll_setAdvParam(  ADV_INTERVAL_35MS, ADV_INTERVAL_35MS,
											 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
											 0,  NULL,
											 BLT_ENABLE_ADV_ALL,
											 ADV_FP_NONE);
	bls_ll_setAdvEnable(1);  //adv enable

	//set scan parameter and scan enable
#if 1
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,	\
							OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
#endif

	blc_ll_setAdvCustomedChannel(39,39,39);

}








extern void usb_handle_irq(void);
extern void proc_button (void);
extern void proc_audio (void);
/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
int main_idle_loop (void)
{


	////////////////////////////////////// BLE entry /////////////////////////////////
	blms_sdk_main_loop();


	////////////////////////////////////// USB  /////////////////////////////////
	#if (APPLICATION_DONGLE)
		usb_handle_irq();

		static u32 tick_bo;
		if (REG_ADDR8(0x125) & BIT(0))
		{
			tick_bo = clock_time ();
		}
		else if (clock_time_exceed (tick_bo, 200000))
		{
			REG_ADDR8(0x125) = BIT(0);
		}
	#endif

	/////////////////////////////////////// HCI ///////////////////////////////////////
//	blc_hci_proc ();



	////////////////////////////////////// UI entry /////////////////////////////////
#if (UI_BUTTON_ENABLE)
	static u8 button_detect_en = 0;
	if(!button_detect_en && clock_time_exceed(0, 1000000)){// proc button 1 second later after power on
		button_detect_en = 1;
	}
	static u32 button_detect_tick = 0;
	if(button_detect_en && clock_time_exceed(button_detect_tick, 5000))
	{
		button_detect_tick = clock_time();
		proc_button();  //button triggers pair & unpair  and OTA
	}
#endif


//	host_pair_unpair_proc();


#if(BLE_MASTER_OTA_ENABLE)

#endif

	return 0;
}




void main_loop (void)
{
	main_idle_loop ();

	if (main_service)
	{
		main_service ();
		main_service = 0;
	}
}





