/********************************************************************************************************
 * @file	 app.c
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

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "application/keyboard/keyboard.h"

#include "vendor/common/tl_audio.h"
#include "vendor/common/blt_led.h"
#include "vendor/common/blt_soft_timer.h"
#include "vendor/common/blt_common.h"

#include "app_ui.h"
// #include "system_ble_task.h"

#if( BLE_IOS_ANCS_ENABLE )
	#include "ancs/ancs.h"
#endif

#if (__PROJECT_8258_BLE_REMOTE__)



#define 	ADV_IDLE_ENTER_DEEP_TIME			60  //60 s
#define 	CONN_IDLE_ENTER_DEEP_TIME			60  //60 s

#define 	MY_DIRECT_ADV_TMIE					2000000


#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_37//BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS//ADV_INTERVAL_1S//ADV_INTERVAL_300MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_35MS//ADV_INTERVAL_1_28_S//ADV_INTERVAL_305MS

#define		MY_RF_POWER_INDEX					RF_POWER_P3p01dBm

#define		BLE_DEVICE_ADDRESS_TYPE 			BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_	own_addr_type_t 	app_own_address_type = OWN_ADDRESS_PUBLIC;





#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		16


#if 0
	MYFIFO_INIT(blt_rxfifo, RX_FIFO_SIZE, RX_FIFO_NUM);
#else
_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};
#endif


#if 0
	MYFIFO_INIT(blt_txfifo, TX_FIFO_SIZE, TX_FIFO_NUM);
#else
	_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
	_attribute_data_retention_	my_fifo_t	blt_txfifo = {
													TX_FIFO_SIZE,
													TX_FIFO_NUM,
													0,
													0,
													blt_txfifo_b,};
#endif




//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8	tbl_advData[] = {
	0x0c, 0x09, 'X','3','0','5','6','#','1','1','2','2','1',
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x03, 0x19, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
	 0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	0x0c, 0x09, 'X','3','0','5','6','#','1','1','2','2','1',
	};


_attribute_data_retention_	int device_in_connection_state;

_attribute_data_retention_	u32 advertise_begin_tick;

_attribute_data_retention_	u32	interval_update_tick;

_attribute_data_retention_	u8	sendTerminate_before_enterDeep = 0;

_attribute_data_retention_	u32	latest_user_event_tick;

_attribute_data_retention_	u32	lowBattDet_tick   = 0;

#if( BLE_IOS_ANCS_ENABLE )
_attribute_data_retention_		u8		bls_needSendSecurityReq = 0;
_attribute_data_retention_		u8		bls_is_ios = 0;
_attribute_data_retention_  	u32 	tick_conn_update_parm = 0;
_attribute_data_retention_  	u32 	tick_findAncsSrv;
_attribute_data_retention_  	u32 	tick_encryptionFinish;

void task_updtConnParm();
void task_findAncsService();
#endif


void 	app_switch_to_indirect_adv(u8 e, u8 *p, int n)
{

	bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
						ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
						0,  NULL,
						MY_APP_ADV_CHANNEL,
						ADV_FP_NONE);

	bls_ll_setAdvEnable(1);  //must: set adv enable
}



void 	ble_remote_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{
	device_in_connection_state = 0;


	if(*p == HCI_ERR_CONN_TIMEOUT){

	}
	else if(*p == HCI_ERR_REMOTE_USER_TERM_CONN){  //0x13

	}
	else if(*p == HCI_ERR_CONN_TERM_MIC_FAILURE){

	}
	else{

	}



#if (BLE_IOS_ANCS_ENABLE)
//	smp_param_peer.paring_enable = SMP_PARING_DISABLE_TRRIGER;
	bls_is_ios = 0;
#endif
#if (BLE_REMOTE_PM_ENABLE)
	 //user has push terminate pkt to ble TX buffer before deepsleep
	if(sendTerminate_before_enterDeep == 1){
		sendTerminate_before_enterDeep = 2;
	}
#endif

	advertise_begin_tick = clock_time();

}

_attribute_ram_code_ void	user_set_rf_power (u8 e, u8 *p, int n)
{
	rf_set_power_level_index (MY_RF_POWER_INDEX);
}


enum{
	NO_NEED_SEND_SEC_REQ = 0,
	NEED_SEND_SEC_REQ = 1,
	ENCRYPTION_RE_CONNECT = 2,
};

enum{
	SYSTEM_TYPE_ANDROID = 0,
	SYSTEM_TYPE_IOS		= 1,
};

void app_setSystemType(u8 isIOS){
	bls_is_ios = isIOS;
}

u8 app_getSystemType(){
	return bls_is_ios;
}

void app_setSendSecReqFlag(u8 en){
	bls_needSendSecurityReq = en;
}

u8 app_getSendSecReqFlag(){
	return bls_needSendSecurityReq;
}

void conn_update_parm_with_latency(void)
{
	u16 curConnLatency = bls_ll_getConnectionLatency();
	u16 curConnInterval = bls_ll_getConnectionInterval();
	if((curConnLatency < 14) || (curConnInterval < 50)){
		bls_l2cap_requestConnParamUpdate(50, 70, 14, 600);
	}
}

void task_encryption_done(){

	tick_encryptionFinish = clock_time() | 0x01;
	app_setSendSecReqFlag(ENCRYPTION_RE_CONNECT);
}


void task_pairing_end(u8 *p)
{
	if(*p)
	{
		blc_ll_setEncryptionBusy(0);   //清除标志位
	}
}


void task_ancs_proc(u16 connHandle, u8 *p)
{
	if(ancsFuncIsEn()){
		ancsStackCallback(p);
	}

	if(app_getSendSecReqFlag() == NO_NEED_SEND_SEC_REQ ){
		rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)p;

		if((req->chanId == 0x04)
			&& (req->opcode == ATT_OP_FIND_BY_TYPE_VALUE_RSP)){///ATT
			app_setSendSecReqFlag(NEED_SEND_SEC_REQ);
			blc_smp_sendSecurityRequest();
			tick_findAncsSrv = 0x00;
		}else{
			if((req->opcode == ATT_OP_ERROR_RSP) ///op_code
				&& (req->data[0] == ATT_OP_FIND_BY_TYPE_VALUE_RSP))			{///err_op_code
				//tick_findAncsSrv = clock_time() | 1;
				//att not found
				app_setSendSecReqFlag(NO_NEED_SEND_SEC_REQ);
			}
		}
	}

	return;
}

void app_curSystemType(){

	if((app_getSendSecReqFlag() == NEED_SEND_SEC_REQ) &&
			(ancsGetConnState() == ANCS_CONNECTION_ESTABLISHED)){
		app_setSystemType(SYSTEM_TYPE_IOS);
	}else{
		app_setSystemType(SYSTEM_TYPE_ANDROID);
	}
}

u8 task_runByGap(u32 *tickRef, u32 tickGap, u8 *retry){

	if(((*tickRef) & 0x01) && (clock_time_exceed((*tickRef), tickGap))){
		(*retry)++;
		(*tickRef) = clock_time() | 0x01;

		return 1;
	}else{
		if((*retry) == 3){
			(*retry) = 0;
			(*tickRef) = 0;
		}

		return 0;
	}
}

void task_findAncsService(){
	_attribute_data_retention_ static u8 ancsRetry = 0;

	if(task_runByGap(&tick_findAncsSrv, 2 * 1000 * 1000, &ancsRetry)){
		ancs_findAncsService();
	}
}

void task_updtConnParm(){
	_attribute_data_retention_ static u8 updtParmRetry = 0;
	_attribute_data_retention_ static u32 gapUpdate_t = 4000000;
	if(task_runByGap(&tick_conn_update_parm, gapUpdate_t, &updtParmRetry)){
		conn_update_parm_with_latency();
		tick_conn_update_parm = clock_time() | 0x01;
		gapUpdate_t = gapUpdate_t + 1000000;
		if(gapUpdate_t > 20000000){///reset the time of checking loop
			gapUpdate_t = 3000000;
		}
	}
}


void task_onceConn(){
	if(blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		return;
	}

	task_updtConnParm();
	task_findAncsService();
	app_curSystemType();

	if((tick_encryptionFinish & 0x01)
		&& (clock_time_exceed(tick_encryptionFinish, 4 * 1000 * 1000))){
		ancsFuncSetEnable(1);
		tick_encryptionFinish = 0x00;
	}
}

void	task_connect (u8 e, u8 *p, int n)
{
	bls_l2cap_requestConnParamUpdate (50, 70, 0, 600);
	bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(2000);

	tick_conn_update_parm = clock_time() | 1;


    //ancs_findAncsService();
    app_setSendSecReqFlag(NO_NEED_SEND_SEC_REQ);
    tick_findAncsSrv = clock_time() | 0x01;
    ancsInit();
    app_setSystemType(SYSTEM_TYPE_ANDROID);

}

int app_conn_param_update_response(u8 id, u16  result)
{
	if(result == CONN_PARAM_UPDATE_ACCEPT){

	}
	else if(result == CONN_PARAM_UPDATE_REJECT){

	}

	return 0;
}

int app_host_event_callback(u32 h, u8 *para, int n){
	u8 event = h & 0xFF;
	switch(event){
		case GAP_EVT_SMP_PARING_BEAGIN:{
#if UART_PRINT_DEBUG_ENABLE
			printf("Pairing begin\n");
#endif
			break;
		}
		case GAP_EVT_SMP_PARING_SUCCESS:{
#if UART_PRINT_DEBUG_ENABLE
			gap_smp_paringSuccessEvt_t* p = (gap_smp_paringSuccessEvt_t*)para;
			printf("Pairing success:bond flg %s\n", p->bonding ?"true":"false");

			if(p->bonding_result){

				printf("save smp key succ\n");

			}
			else{
				printf("save smp key failed\n");
			}
#endif
			break;
		}
		case GAP_EVT_SMP_PARING_FAIL:{
			task_pairing_end(para);

#if UART_PRINT_DEBUG_ENABLE
			gap_smp_paringFailEvt_t* p = (gap_smp_paringFailEvt_t*)para;
			printf("Pairing failed:rsn:0x%x\n", p->reason);
#endif
			break;
		}
		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:{

			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
#if UART_PRINT_DEBUG_ENABLE
			printf("Connection encryption done\n");
#endif
			task_encryption_done();
			if(p->re_connect == SMP_STANDARD_PAIR){ //first paring

			}
			else if(p->re_connect == SMP_FAST_CONNECT){ //auto connect

			}

			break;
		}
		default:
			break;
	}
	return 0;
}

void	task_conn_update_req (u8 e, u8 *p, int n)
{

}

void	task_conn_update_done (u8 e, u8 *p, int n)
{
	u16 newconnInterval;
	u16 newconnLatency;
	u16 newTimeout;
	newconnInterval = p[0]|p[1]<<8;
	newconnLatency = p[2]|p[3]<<8;
	newTimeout = p[4]|p[5]<<8;
	Cb_BleOnUpdateConnParamsResult(1, newconnInterval, newconnLatency);

}


void  ble_remote_set_sleep_wakeup (u8 e, u8 *p, int n)
{
	//if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * CLOCK_16M_SYS_TIMER_CLK_1MS){  //suspend time > 30ms.add gpio wakeup
		bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
	//}
}

void user_init_normal(void)
{

	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
	random_generator_init();  //this is must



	/*****************************************************************************************
	 Note: battery check must do before any flash write/erase operation, cause flash write/erase
		   under a low or unstable power supply will lead to error flash operation

		   Some module initialization may involve flash write/erase, include: OTA initialization,
				SMP initialization, ..
				So these initialization must be done after  battery check
	*****************************************************************************************/
	#if (0)  //battery check must do before OTA relative operation
		if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
			app_battery_power_check(VBAT_ALRAM_THRES_MV + 200);  //2.2 V
		}
		else{
			app_battery_power_check(VBAT_ALRAM_THRES_MV);  //2.0 V
		}
	#endif



////////////////// BLE stack initialization ////////////////////////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

	#if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
		app_own_address_type = OWN_ADDRESS_PUBLIC;
	#elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
		app_own_address_type = OWN_ADDRESS_RANDOM;
		blc_ll_setRandomAddr(mac_random_static);
	#endif

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initAdvertising_module(mac_public); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,
	blc_ll_initPowerManagement_module();        //pm module:      	 optional



	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	extern void my_att_init ();
	my_att_init (); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	#if (BLE_REMOTE_SECURITY_ENABLE)
		blc_smp_peripheral_init();

#if(BLE_IOS_ANCS_ENABLE)
		blc_smp_configSecurityRequestSending(SecReq_NOT_SEND, SecReq_NOT_SEND, 0);
		blc_gap_registerHostEventHandler( app_host_event_callback );
		blc_gap_setEventMask(
		GAP_EVT_MASK_SMP_PARING_BEAGIN |
		GAP_EVT_MASK_SMP_PARING_SUCCESS |
		GAP_EVT_MASK_SMP_PARING_FAIL |
		GAP_EVT_ATT_EXCHANGE_MTU |
		GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );
#else
		//Hid device on android7.0/7.1 or later version
		// New paring: send security_request immediately after connection complete
		// reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request.
		blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
#endif
		#else
		blc_smp_setSecurityLevel(No_Security);
	#endif




///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));




	////////////////// config adv packet /////////////////////
#if (BLE_REMOTE_SECURITY_ENABLE)
	u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
	smp_param_save_t  bondInfo;
	if(bond_number)   //at least 1 bonding device exist
	{
		bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

	}

	if(bond_number)   //set direct adv
	{
		//set direct adv
		u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
										bondInfo.peer_addr_type,  bondInfo.peer_addr,
										MY_APP_ADV_CHANNEL,
										ADV_FP_NONE);
		if(status != BLE_SUCCESS) { write_reg8(0x40002, 0x11); 	while(1); }  //debug: adv setting err

		//it is recommended that direct adv only last for several seconds, then switch to indirect adv
		bls_ll_setAdvDuration(MY_DIRECT_ADV_TMIE, 1);
		bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &app_switch_to_indirect_adv);

	}
	else   //set indirect adv
#endif
	{
		u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
										 0,  NULL,
										 MY_APP_ADV_CHANNEL,
										 ADV_FP_NONE);
		if(status != BLE_SUCCESS) { write_reg8(0x40002, 0x11); 	while(1); }  //debug: adv setting err
	}

	bls_ll_setAdvEnable(1);  //adv enable


	//set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
	user_set_rf_power(0, 0, 0);
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &user_set_rf_power);



	//ble event call back
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &ble_remote_terminate);


	bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_REQ, &task_conn_update_req);
	bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_UPDATE, &task_conn_update_done);

	blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);

	///////////////////// Power Management initialization///////////////////
#if(BLE_REMOTE_PM_ENABLE)
	blc_ll_initPowerManagement_module();

	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(100, 100);
		blc_pm_setDeepsleepRetentionEarlyWakeupTiming(800);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &ble_remote_set_sleep_wakeup);
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif


#if (BLE_REMOTE_OTA_ENABLE)
	////////////////// OTA relative ////////////////////////
	bls_ota_clearNewFwDataArea(); //must
	bls_ota_registerStartCmdCb(app_enter_ota_mode);
	//bls_ota_registerResultIndicateCb(app_debug_ota_result);  //debug
#endif

#if(BLE_IOS_ANCS_ENABLE)
	extern void blc_l2cap_reg_att_cli_hander(void *p);
	blc_l2cap_reg_att_cli_hander(task_ancs_proc);
#endif
	advertise_begin_tick = clock_time();

}




_attribute_ram_code_ void user_init_deepRetn(void)
{
#if (PM_DEEPSLEEP_RETENTION_ENABLE)
	blc_ll_initBasicMCU();   //mandatory
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	blc_ll_recoverDeepRetention();

	DBG_CHN0_HIGH;    //debug

	if(!OS_ENABLE) irq_enable();

#endif
}


/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
u32 tick_loop;
void main_loop (void){
	tick_loop ++;
}

void prv_callback(){
#if( BLE_IOS_ANCS_ENABLE )
	task_onceConn();
	ancsHaveNews();
#endif
}



#endif  //end of __PROJECT_8267_BLE_REMOTE__ || __PROJECT_8261_BLE_REMOTE__ || __PROJECT_8269_BLE_REMOTE__
