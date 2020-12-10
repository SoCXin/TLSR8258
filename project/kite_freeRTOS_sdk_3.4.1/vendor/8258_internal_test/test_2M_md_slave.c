/********************************************************************************************************
 * @file     test_2M_md_slave.c
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     May. 10, 2018
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
#include "app.h"
#include <stack/ble/ble.h>
#include "tl_common.h"
#include "drivers.h"
#include "app_config.h"
#include "vendor/common/blt_led.h"
#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"
#include "vendor/common/blt_soft_timer.h"
#include "vendor/common/blt_common.h"



#if (INTER_TEST_MODE == TEST_2M_SLAVE_CONN_MD)



#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		16

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		16


#define FEATURE_PM_ENABLE								0
#define FEATURE_DEEPSLEEP_RETENTION_ENABLE				0


_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};


_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_txfifo = {
												TX_FIFO_SIZE,
												TX_FIFO_NUM,
												0,
												0,
												blt_txfifo_b,};


_attribute_data_retention_	u8 		key_type;
_attribute_data_retention_	int 	key_not_released;

_attribute_data_retention_  int start_2m_pkt_notify_pendingTick;

_attribute_data_retention_  int start_2m_pkt_notify_test = 0;
_attribute_data_retention_  int ota_is_working = 0;
_attribute_data_retention_  int currRcvdSeqNo = 0;
_attribute_data_retention_  int seqNoSend = 0;
_attribute_data_retention_  u8 test_data[128]={
	0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x12, 0x89, 0x40, 0x83, 0x51, 0xBB, 0xB9, 0xA0, 0x1E, 0x39,
	0x70, 0xA2, 0xB3, 0x21, 0x1B, 0x12, 0xCF, 0x21, 0x92, 0x9A, 0x02, 0xF8, 0xB1, 0x9B, 0xB4, 0x28,
	0xB8, 0x06, 0x11, 0xB9, 0x8A, 0x52, 0x81, 0x85, 0x01, 0x0C, 0x10, 0xA9, 0x35, 0x29, 0xA9, 0xA6,
	0x90, 0x1B, 0x9B, 0x81, 0x1B, 0x70, 0x3A, 0xF2, 0x98, 0x82, 0x80, 0x91, 0x93, 0xB3, 0x99, 0xA4,
	0x26, 0x38, 0x63, 0x90, 0xF3, 0x0B, 0xB0, 0x02, 0x28, 0x29, 0xCB, 0xA2, 0x86, 0x9B, 0x1B, 0x51,
	0xB8, 0x16, 0x09, 0xE0, 0x84, 0x21, 0x90, 0x89, 0x89, 0x8F, 0xB1, 0x1C, 0x89, 0x02, 0x19, 0x11,
	0x10, 0x24, 0xD0, 0xAB, 0xF2, 0x3B, 0x02, 0x22, 0x49, 0xE9, 0x01, 0x0B, 0x91, 0x16, 0xB3, 0x93,
	0x1D, 0x00, 0x15, 0x81, 0x1B, 0x22, 0x12, 0x10, 0xBE, 0x0B, 0xBD, 0x13, 0x10, 0xC8, 0x01, 0xEE
};

_attribute_data_retention_  int phy_set_flg = 0;
_attribute_data_retention_  int phy_update_test_tick = 0;

static u16 vk_consumer_map[16] = {
	MKEY_VOL_UP,
	MKEY_VOL_DN,
	MKEY_MUTE,
	MKEY_CHN_UP,
	MKEY_CHN_DN,
	MKEY_POWER,
	MKEY_AC_SEARCH,
	MKEY_RECORD,
	MKEY_PLAY,
	MKEY_PAUSE,
	MKEY_STOP,
	MKEY_FAST_FORWARD,
	MKEY_FAST_FORWARD,
	MKEY_AC_HOME,
	MKEY_AC_BACK,
	MKEY_MENU,
};

static void led_onoff(bool on_or_off)
{
	u8 onoff = on_or_off ? 1:0;
	gpio_set_output_en(GPIO_LED, onoff);
	gpio_write(GPIO_LED, onoff);
}

int myC2SWrite(void * p)
{
	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
	u32 seqNoRcv = (u32) req->dat[0] | req->dat[1]<<8 | req->dat[2] << 16 | req->dat[3]<<32;
	//printf("%d[%d]\n", seqNoRcv, currRcvdSeqNo);

	u8 test_en_flg = req->dat[4]; //1:start write seqNo check; 0: stop write seqNo check;

	if(test_en_flg == 0){
		currRcvdSeqNo = 0;
		ota_is_working = 0;
	}
	else if(currRcvdSeqNo == seqNoRcv){
		ota_is_working = 1;
		currRcvdSeqNo++;
	}
	else{
		printf("ERR:seq lost[rcv:0x%x][cnt:0x%x\n", seqNoRcv, currRcvdSeqNo);
		write_reg8(0x40000, 0x77);
		irq_disable();
		while(1);
		write_reg8(0x40000, 0x22);
	}

	return 1;
}


void  ble_remote_set_sleep_wakeup (u8 e, u8 *p, int n)
{
	if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * CLOCK_16M_SYS_TIMER_CLK_1MS){  //suspend time > 30ms.add gpio wakeup
		bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
	}
}

void key_change_proc(void)
{
	u8 key0 = kb_event.keycode[0];
	u8 key_value;
	u8 key_buf[8] = {0,0,0,0,0,0,0,0};

	key_not_released = 1;
	if (kb_event.cnt >= 2){   //two or more key press, do  not process

	}
	else if(kb_event.cnt == 1){
		key_value = key0;
		if (key0 == VOICE){
			start_2m_pkt_notify_test ^= 1;

			////////// clr /////////////////
			seqNoSend = 0;
			*(u32*)test_data = seqNoSend;
			////////////////////////////////

			if(start_2m_pkt_notify_test){
				printf("Start 2m notify test\n");
				test_data[4] = 1;
				start_2m_pkt_notify_pendingTick = clock_time()|1;
				led_onoff(LED_ON_LEVAL);
			}
			else{
				printf("Stop 2m notify test\n");
				test_data[4] = 0;
				start_2m_pkt_notify_pendingTick = 0;
				bls_att_pushNotifyData (SPP_SERVER_TO_CLIENT_DP_H, (u8*)test_data, 5);
				led_onoff(!LED_ON_LEVAL);
			}
		}
		else if(key_value >= 0xf0 ){
			key_type = CONSUMER_KEY;
			u16 consumer_key = vk_consumer_map[key_value & 0x0f];
			bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			printf("CK:0x%x pressed\n", consumer_key);
		}
		else
		{
			key_type = KEYBOARD_KEY;
			key_buf[2] = key_value;
			bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			printf("KK:0x%x pressed\n", key_value);
		}
	}
	else   //kb_event.cnt == 0,  key release
	{
		key_not_released = 0;
		if(key_type == CONSUMER_KEY)
		{
			u16 consumer_key = 0;
			bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			printf("CK:released\n");
		}
		else if(key_type == KEYBOARD_KEY)
		{
			key_buf[2] = 0;
			bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
			printf("KK:released\n");
		}
	}
}

_attribute_data_retention_ static int gpioWakeup_keyProc_cnt = 0;
_attribute_data_retention_ static u32 keyScanTick = 0;
void proc_keyboard (u8 e, u8 *p, int n)
{
	//when key press gpio wakeup suspend, proc keyscan at least GPIO_WAKEUP_KEYPROC_CNT times
	//regardless of 8000 us interval
	if(e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP){
		gpioWakeup_keyProc_cnt = GPIO_WAKEUP_KEYPROC_CNT;
	}
	else if(gpioWakeup_keyProc_cnt){
		gpioWakeup_keyProc_cnt --;
	}

	if(gpioWakeup_keyProc_cnt || clock_time_exceed(keyScanTick, 8000)){
		keyScanTick = clock_time();
	}
	else{
		return;
	}

	kb_event.keycode[0] = 0;
	int det_key = kb_scan_key (0, 1);

	if (det_key){
		key_change_proc();
	}
}


void function_s2M_md_test_mainloop(void)
{
	proc_keyboard (0,0, 0);

	if(start_2m_pkt_notify_pendingTick && clock_time_exceed(start_2m_pkt_notify_pendingTick, 1000000)){ // >1s
		if(phy_set_flg){
			if( BLE_SUCCESS == bls_att_pushNotifyData (SPP_SERVER_TO_CLIENT_DP_H, (u8*)test_data, sizeof(test_data))){
				seqNoSend++;
				*(u32*)test_data = seqNoSend;
			}
		}
	}


	if(phy_update_test_tick && clock_time_exceed(phy_update_test_tick, 1000000)){ //T > 1s
		phy_update_test_tick = 0;

		if(!phy_set_flg){
			printf("After conn 1s, if not receive M's PHY exchange pkt, S send PHY exchange req to M.\n");
			blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_2M, 	 PHY_PREFER_2M,    CODED_PHY_PREFER_NONE);
			phy_set_flg = 1;
		}
	}

	#if (FEATURE_DEEPSLEEP_RETENTION_ENABLE)
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	int user_task_flg = scan_pin_need || key_not_released || ota_is_working;

	if(user_task_flg){
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
		extern int  key_matrix_same_as_last_cnt;
		if(!ota_is_working && key_matrix_same_as_last_cnt > 5){  //key matrix stable can optize
			bls_pm_setManualLatency(3);
		}
		else{
			bls_pm_setManualLatency(0);  //latency off: 0
		}
	}

	if(start_2m_pkt_notify_test){
		bls_pm_setSuspendMask (SUSPEND_DISABLE);
	}

}





void	task_connect (u8 e, u8 *p, int n)
{
	printf("connected\n");

	phy_update_test_tick = clock_time()|1;

	bls_l2cap_requestConnParamUpdate (8, 8, 99, 400);  //interval=10ms latency=99 timeout=4s
	bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(1000);

	currRcvdSeqNo = 0;
	seqNoSend = 0;
	ota_is_working = 0;

	phy_set_flg = 0;
}

volatile u8 A_dis_conn_rsn;
void	task_terminate (u8 e, u8 *p, int n)
{
	printf("terminate rsn: 0x%x\n", *p);
	led_onoff(!LED_ON_LEVAL);
}

void callback_phy_update_complete_event (u8 e, u8 *p, int n)
{
	printf("PHY update: 0x%x\n", *p);

	if(!phy_set_flg){
		phy_set_flg = 1;
		printf("S Rcv M's PHY update\n");
	}
}

int AA_dbg_suspend;
void  func_suspend_enter (u8 e, u8 *p, int n)
{
	AA_dbg_suspend ++;
}

#define		MY_RF_POWER_INDEX					RF_POWER_P3p01dBm

_attribute_ram_code_ void  func_suspend_exit (u8 e, u8 *p, int n)
{
	rf_set_power_level_index (MY_RF_POWER_INDEX);
}









int app_host_event_callback (u32 h, u8 *para, int n)
{
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PARING_BEAGIN:
		{
			printf("Pairing begin\n");
		}
		break;

		case GAP_EVT_SMP_PARING_SUCCESS:
		{
			gap_smp_paringSuccessEvt_t* p = (gap_smp_paringSuccessEvt_t*)para;
			printf("Pairing success:bond flg %s\n", p->bonding ?"true":"false");

			if(p->bonding_result){
				printf("save smp key succ\n");
			}
			else{
				printf("save smp key failed\n");
			}
		}
		break;

		case GAP_EVT_SMP_PARING_FAIL:
		{
			gap_smp_paringFailEvt_t* p = (gap_smp_paringFailEvt_t*)para;
			printf("Pairing failed:rsn:0x%x\n", p->reason);
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
			printf("Connection encryption done\n");

			if(p->re_connect == SMP_STANDARD_PAIR){  //first paring

			}
			else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect

			}
		}
		break;

		case GAP_EVT_SMP_TK_DISPALY:
		{
			char pc[7];
			u32 pinCode = *(u32*)para;
			sprintf(pc, "%d", pinCode);
			printf("TK display:%s\n", pc);
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
		{
			printf("TK Request passkey\n");
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_OOB:
		{
			printf("TK Request OOB\n");
		}
		break;

		case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
		{
			char pc[7];
			u32 pinCode = *(u32*)para;
			sprintf(pc, "%d", pinCode);
			printf("TK numeric comparison:%s\n", pc);
		}
		break;

		default:
		break;
	}

	return 0;
}

















extern void	my_att_init(void);


void function_s2M_md_test_init_normal(void)
{

	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
	random_generator_init();  //this is must



	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

	rf_set_power_level_index (MY_RF_POWER_INDEX);

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();   //mandatory
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master

	blc_ll_initAdvertising_module(mac_public); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,

	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	my_att_init(); 		//gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization

	blc_ll_init2MPhyCodedPhy_feature();

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_2"
	blc_smp_setSecurityLevel(Unauthenticated_Paring_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Paring_with_Encryption)
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABLITY_NO_IN_NO_OUT);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_DISPALY				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );


///////////////////// USER application initialization ///////////////////
	u8 tbl_advData[] = {
		 0x08, 0x09, 't', 'e', 's', 't', 'S', 'M', 'P',
		#if (1) //We'd better add this, because for some smartphones, the system may be forbidden to connect
			 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
			 0x03, 0x19, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
			 0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
		#endif
		};
	u8	tbl_scanRsp [] = {
			 0x08, 0x09, 't', 'e', 's', 't', 'S', 'M', 'P',
		};
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));

	bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_30MS,
						ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
						0,  NULL,  BLT_ENABLE_ADV_37, ADV_FP_NONE);

	bls_ll_setAdvEnable(1);  //adv enable

	//ble event call back
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &task_terminate);
	bls_app_registerEventCallback (BLT_EV_FLAG_PHY_UPDATE, &callback_phy_update_complete_event);


#if(FEATURE_PM_ENABLE)
	blc_ll_initPowerManagement_module();

	#if (FEATURE_DEEPSLEEP_RETENTION_ENABLE)
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(50, 50);
		blc_pm_setDeepsleepRetentionEarlyWakeupTiming(400);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &ble_remote_set_sleep_wakeup);
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &func_suspend_exit);
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif

	/////////// keyboard gpio wakeup init ////////
	u32 pin[] = KB_DRIVE_PINS;
	for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
	{
		cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin pad high wakeup deepsleep
	}

	bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);

}



_attribute_ram_code_ void function_s2M_md_test_init_deepRetn(void)
{
#if (FEATURE_DEEPSLEEP_RETENTION_ENABLE)
	blc_ll_initBasicMCU();   //mandatory
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	blc_ll_recoverDeepRetention();

	irq_enable();

	DBG_CHN0_HIGH;    //debug

	/////////// keyboard gpio wakeup init ////////
	u32 pin[] = KB_DRIVE_PINS;
	for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
	{
		cpu_set_gpio_wakeup (pin[i], Level_High, 1);  //drive pin pad high wakeup deepsleep
	}
#endif
}




#endif
