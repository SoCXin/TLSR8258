/********************************************************************************************************
 * @file     test_2M_md_master.c
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
#include "vendor/common/blt_common.h"




#if (INTER_TEST_MODE == TEST_2M_MASTER_CONN_MD)

	#define SMP_PENDING							1   //security management
	int	app_host_smp_pending = 0;


#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		16

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		16


MYFIFO_INIT(blt_rxfifo, RX_FIFO_SIZE, RX_FIFO_NUM);
MYFIFO_INIT(blt_txfifo, TX_FIFO_SIZE, TX_FIFO_NUM);

#define FEATURE_PM_ENABLE								0
#define FEATURE_DEEPSLEEP_RETENTION_ENABLE				0

static u32 master_connected_led_on;
static u32 host_update_conn_param_req;
static u16 host_update_conn_min;
static u16 host_update_conn_latency;
static u16 host_update_conn_timeout;
static u32 connect_event_occurTick;
static u32 start_2m_pkt_write_test;
static u32 start_2m_pkt_write_pendingTick;
static u32 dongle_pairing_enable;
static u32 dongle_unpair_enable;

// connection character device information
static u8  cur_conn_mac_adrType;
static u8  cur_conn_mac_addr[6];
static u16 cur_conn_handle;





#if (BLE_HOST_SMP_ENABLE)
	int app_host_smp_finish (void)  //smp finish callback
	{
		app_host_smp_pending = 0;  //no sdp
		printf("smp finish\n");
		return 0;
	}
#endif


_attribute_data_retention_	u32 currRcvdSeqNo = 0;
_attribute_data_retention_  int seqNoSend = 0;
_attribute_data_retention_  u8 test_data[20]={
	0x00, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x12, 0x89, 0x40, 0x83, 0x51, 0xBB, 0xB9, 0xA0, 0x1E, 0x39,
	0x70, 0xA2, 0xB3, 0x21
};

#if (1) //button
	#define MAX_BTN_SIZE			2
	#define BTN_VALID_LEVEL			0
	#define BTN_PAIR				0x01
	#define BTN_UNPAIR				0x02

	u32 ctrl_btn[] = {SW1_GPIO, SW2_GPIO};
	u8 btn_map[MAX_BTN_SIZE] = {BTN_PAIR, BTN_UNPAIR};

	typedef	struct{
		u8 	cnt;				//count button num
		u8 	btn_press;
		u8 	keycode[MAX_BTN_SIZE];			//6 btn
	}vc_data_t;
	vc_data_t vc_event;

	typedef struct{
		u8  btn_history[4];		//vc history btn save
		u8  btn_filter_last;
		u8	btn_not_release;
		u8 	btn_new;					//new btn  flag
	}btn_status_t;
	btn_status_t 	btn_status;

	u8 btn_debounce_filter(u8 *btn_v)
	{
		u8 change = 0;

		for(int i=3; i>0; i--){
			btn_status.btn_history[i] = btn_status.btn_history[i-1];
		}
		btn_status.btn_history[0] = *btn_v;

		if(  btn_status.btn_history[0] == btn_status.btn_history[1] && btn_status.btn_history[1] == btn_status.btn_history[2] && \
			btn_status.btn_history[0] != btn_status.btn_filter_last ){
			change = 1;

			btn_status.btn_filter_last = btn_status.btn_history[0];
		}
		return change;
	}

	u8 vc_detect_button(int read_key)
	{
		u8 btn_changed, i;
		memset(&vc_event,0,sizeof(vc_data_t));			//clear vc_event
		//vc_event.btn_press = 0;

		for(i=0; i<MAX_BTN_SIZE; i++){
			if(BTN_VALID_LEVEL != !gpio_read(ctrl_btn[i])){
				vc_event.btn_press |= BIT(i);
			}
		}

		btn_changed = btn_debounce_filter(&vc_event.btn_press);

		if(btn_changed && read_key){
			for(i=0; i<MAX_BTN_SIZE; i++){
				if(vc_event.btn_press & BIT(i)){
					vc_event.keycode[vc_event.cnt++] = btn_map[i];
				}
			}
			return 1;
		}
		return 0;
	}

	void proc_button (void)
	{
		int det_key = vc_detect_button (1);

		if (det_key)  //key change: press or release
		{
			u8 key0 = vc_event.keycode[0];

			if(vc_event.cnt == 1) //one key press
			{
				if(key0 == BTN_PAIR)
				{
					dongle_pairing_enable = 1;

					if(master_connected_led_on) //test DLE, write cmd data
					{
						start_2m_pkt_write_test ^= 1;

						////////// clr /////////////////
						seqNoSend = 0;
						*(u32*)test_data = seqNoSend;
						////////////////////////////////

						if(start_2m_pkt_write_test){
							printf("Start 2m write test\n");
							test_data[4] = 1;
							start_2m_pkt_write_pendingTick = clock_time()|1;
						}
						else{
							printf("Stop 2m write test\n");
							test_data[4] = 0;
							blc_gatt_pushWriteComand(BLM_CONN_HANDLE, SPP_CLIENT_TO_SERVER_DP_H, test_data, 5);
						}
					}
				}
				else if(key0 == BTN_UNPAIR)
				{
					dongle_unpair_enable = 1;
				}
			}
			else
			{  //release
				if(dongle_pairing_enable)
				{
					dongle_pairing_enable = 0;
				}

				if(dongle_unpair_enable)
				{
					dongle_unpair_enable = 0;
				}
			}
		}
	}
#endif



int app_l2cap_handler (u16 conn_handle, u8 *raw_pkt)
{
	//l2cap data packeted, make sure that user see complete l2cap data
	rf_packet_l2cap_t *ptrL2cap = blm_l2cap_packet_pack (conn_handle, raw_pkt);
	if (!ptrL2cap)
		return 0;

	//l2cap data channel id, 4 for att, 5 for signal, 6 for smp
	if(ptrL2cap->chanId == L2CAP_CID_ATTR_PROTOCOL)  //att data
	{
		rf_packet_att_t *pAtt = (rf_packet_att_t*)ptrL2cap;
		u16 attHandle = pAtt->handle0 | pAtt->handle1<<8;

		if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //slave handle notify
		{
			if(attHandle == SPP_SERVER_TO_CLIENT_DP_H){

				u32 seqNoRcv = (u32) pAtt->dat[0] | pAtt->dat[1]<<8 | pAtt->dat[2] << 16 | pAtt->dat[3]<<32;
				u8 test_en_flg = pAtt->dat[4]; //1:start notify seqNo check; 0: stop notify seqNo check;
				if(test_en_flg == 0){
					currRcvdSeqNo = 0;
				}
				else if(currRcvdSeqNo == seqNoRcv){
					currRcvdSeqNo++;
				}
				else{
					printf("ERR:seq lost[rcv:0x%x][cnt:0x%x\n", seqNoRcv, currRcvdSeqNo);
					write_reg8(0x40000, 0x77);
					irq_disable();
					while(1);
					write_reg8(0x40000, 0x22);
				}
			}

		}
	}
	else if(ptrL2cap->chanId == L2CAP_CID_SIG_CHANNEL)  //signal
	{
		if(ptrL2cap->opcode == L2CAP_CMD_CONN_UPD_PARA_REQ)  //slave send conn param update req on l2cap
		{
			rf_packet_l2cap_connParaUpReq_t  * req = (rf_packet_l2cap_connParaUpReq_t *)ptrL2cap;

			u32 interval_us = req->min_interval*1250;  //1.25ms unit
			u32 timeout_us = req->timeout*10000; //10ms unit
			u32 long_suspend_us = interval_us * (req->latency+1);

			//interval < 200ms, long suspend < 11S, interval * (latency +1)*2 <= timeout
			if( interval_us < 200000 && long_suspend_us < 20000000 && (long_suspend_us*2<=timeout_us) )
			{
				//when master host accept slave's conn param update req, should send a conn param update response on l2cap
				//with CONN_PARAM_UPDATE_ACCEPT; if not accpet,should send  CONN_PARAM_UPDATE_REJECT
				blc_l2cap_SendConnParamUpdateResponse(conn_handle, req->id, CONN_PARAM_UPDATE_ACCEPT);  //send SIG Connection Param Update Response

				printf("send SIG Connection Param Update accept\n");

				//if accept, master host should mark this, add will send  update conn param req on link layer later set a flag here, then send update conn param req in mainloop
				host_update_conn_param_req = clock_time() | 1 ; //in case zero value
				host_update_conn_min = req->min_interval;  //backup update param
				host_update_conn_latency = req->latency;
				host_update_conn_timeout = req->timeout;
			}
			else
			{
				blc_l2cap_SendConnParamUpdateResponse(conn_handle, req->id, CONN_PARAM_UPDATE_REJECT);  //send SIG Connection Param Update Response
				printf("send SIG Connection Param Update reject\n");
			}
		}
	}
	else if(ptrL2cap->chanId == L2CAP_CID_SMP) //smp
	{
		#if (BLE_HOST_SMP_ENABLE)
			if(app_host_smp_pending == SMP_PENDING)
			{
				blm_host_smp_handler(conn_handle, (u8 *)ptrL2cap);
			}

		#endif
	}
	return 0;
}

//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////
int controller_event_callback (u32 h, u8 *p, int n)
{
	if (h &HCI_FLAG_EVENT_BT_STD)		//ble controller hci event
	{
		u8 evtCode = h & 0xff;

		//------------ disconnect -------------------------------------
		if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
		{
			event_disconnection_t	*pd = (event_disconnection_t *)p;

			//terminate reason//connection timeout
			if(pd->reason == HCI_ERR_CONN_TIMEOUT){
			}
			//peer device(slave) send terminate cmd on link layer
			else if(pd->reason == HCI_ERR_REMOTE_USER_TERM_CONN){
			}
			//master host disconnect( blm_ll_disconnect(current_connHandle, HCI_ERR_REMOTE_USER_TERM_CONN) )
			else if(pd->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){
			}
			 //master create connection, send conn_req, but did not received acked packet in 6 connection event
			else if(pd->reason == HCI_ERR_CONN_FAILED_TO_ESTABLISH){ //send connection establish event to host(telink defined event)
			}
			else{
			}

			printf("----- terminate rsn: 0x%x -----\n", pd->reason);

			//led show none connection state
			if(master_connected_led_on){
				master_connected_led_on = 0;
				gpio_write(GPIO_LED_WHITE, LED_ON_LEVAL);   //white on
				gpio_write(GPIO_LED_RED, !LED_ON_LEVAL);    //red off
			}

			connect_event_occurTick = 0;
			host_update_conn_param_req = 0; //when disconnect, clear update conn flag
			cur_conn_handle = 0;  //when disconnect, clear conn handle

			app_host_smp_pending = 0;
			start_2m_pkt_write_test = 0;
			start_2m_pkt_write_pendingTick = 0;

			//should set scan mode again to scan slave adv packet
			blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
			blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

		}
#if (BLE_HOST_SMP_ENABLE)
		else if(evtCode == HCI_EVT_ENCRYPTION_CHANGE)
		{
			event_enc_change_t *pe = (event_enc_change_t *)p;
			blm_smp_encChangeEvt(pe->status, pe->handle, pe->enc_enable);
		}
		else if(evtCode == BLM_CONN_ENC_REFRESH)
		{
			event_enc_refresh_t *pe = (event_enc_refresh_t *)p;
			blm_smp_encChangeEvt(pe->status, pe->handle, 1);
		}
#endif
		else if(evtCode == HCI_EVT_LE_META)
		{
			u8 subEvt_code = p[0];

			//------hci le event: le connection establish event---------------------------------
			if(subEvt_code == HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)  //connection establish(telink private event)
			{
				event_connection_complete_t *pCon = (event_connection_complete_t *)p;

				if (pCon->status == BLE_SUCCESS)	// status OK
				{
					printf("----- connected -----\n");

					//led show connection state
					master_connected_led_on = 1;
					gpio_write(GPIO_LED_RED, LED_ON_LEVAL);     //red on
					gpio_write(GPIO_LED_WHITE, !LED_ON_LEVAL);  //white off

					cur_conn_handle = pCon->handle;   //mark conn handle, in fact this equals to BLM_CONN_HANDLE
					connect_event_occurTick = clock_time()|1;

					#if (BLE_HOST_SMP_ENABLE)
						app_host_smp_pending = SMP_PENDING; //pair & security first
						//save current connect address type and address
						cur_conn_mac_adrType = pCon->peer_adr_type;
						memcpy(cur_conn_mac_addr, pCon->mac, 6);
					#endif

					/////////////// test seq no clr ////////////////////
					seqNoSend = 0;
					currRcvdSeqNo = 0;
				}
			}
			//--------hci le event: le adv report event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
			{
				//after controller is set to scan state, it will report all the adv packet it received by this event
				event_adv_report_t *pa = (event_adv_report_t *)p;
				s8 rssi = pa->data[pa->len];

				//printf("LE advertising report (rssi:%ddb, len:%d):\n", rssi, pa->len+11);
				//array_printf(p, (pa->len + 11));

				 //if previous connection smp&sdp not finish, can not create a new connection
				if(app_host_smp_pending){
					return 1;
				}

				int master_auto_connect = 0;
				int user_manual_paring = 0;

				#if (BLE_HOST_SMP_ENABLE)
					master_auto_connect = tbl_bond_slave_search(pa->adr_type, pa->mac);
				#endif

				user_manual_paring = dongle_pairing_enable && (rssi > -56);  //button trigger pairing(rssi threshold, short distance)
				if(master_auto_connect || user_manual_paring)
				{
					//send create connection cmd to controller, trigger it switch to initiating state, after this cmd,
					//controller will scan all the adv packets it received but not report to host, to find the specified
					//device(adr_type & mac), then send a connection request packet after 150us, enter to connection state
					// and send a connection complete event(HCI_SUB_EVT_LE_CONNECTION_COMPLETE)
					ble_sts_t sta = blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, INITIATE_FP_ADV_SPECIFY,  \
											 pa->adr_type, pa->mac, BLE_ADDR_PUBLIC, \
											 CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 0, CONN_TIMEOUT_4S, \
											 0, 0xFFFF);
					if(sta){
						printf("conn err:0x%x\n", sta);
					}
				}
			}
			//------hci le event: le phy update complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_PHY_UPDATE_COMPLETE)	// connection update
			{
				//2 situation can trigger this event:
				//   1) master host trigger: by calling API  blc_ll_setPhy(...)
				//   2) peer slave device trigger: send "LL_PHY_REQ" on linklayer
				//when update take effect, controller send update complete event to host

				hci_le_phyUpdateCompleteEvt_t *pEvt = (hci_le_phyUpdateCompleteEvt_t *)p;
				printf("----- PHY update(status:%d): -----\n", pEvt->status);
				printf("tx_phy: %d\n", pEvt->tx_phy);
				printf("rx_phy: %d\n", pEvt->rx_phy);
			}
		}
	}
	return 0;
}

void function_m2M_md_test_init_normal(void)
{
	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
	random_generator_init();  //this is must


	u8  mac_public[6];
	u8  mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initScanning_module(mac_public); 	//scan module: 		 mandatory for BLE master,
	blc_ll_initInitiating_module();			//initiate module: 	 mandatory for BLE master,
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initMasterRoleSingleConn_module();			//master module: 	 mandatory for BLE master,

	rf_set_power_level_index (RF_POWER_P3p01dBm);

	////// Host Initialization  //////////
	blc_gap_central_init();										//gap initialization
	blc_l2cap_register_handler (app_l2cap_handler);    			//l2cap initialization
	blc_hci_registerControllerEventHandler(controller_event_callback); //controller hci event to host all processed in this func

	#if (BLE_HOST_SMP_ENABLE)
		blm_smp_configParingSecurityInfoStorageAddr(FLASH_ADR_PARING);
		blm_smp_registerSmpFinishCb(app_host_smp_finish);
		blc_smp_central_init();
		//SMP trigger by master
		blm_host_smp_setSecurityTrigger(MASTER_TRIGGER_SMP_FIRST_PAIRING | MASTER_TRIGGER_SMP_AUTO_CONNECT);
	#else
		//NO SMP process
		blc_smp_setSecurityLevel(No_Security);
	#endif


	blc_ll_init2MPhyCodedPhy_feature();


	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE | HCI_EVT_MASK_ENCRYPTION_CHANGE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(  HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
							    | HCI_LE_EVT_MASK_ADVERTISING_REPORT \
							    | HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE \
							    | HCI_LE_EVT_MASK_DATA_LENGTH_CHANGE \
							    | HCI_LE_EVT_MASK_PHY_UPDATE_COMPLETE \
							    | HCI_LE_EVT_MASK_CONNECTION_ESTABLISH ); //connection establish: telink private event

	//ATT initialization
	//blc_att_setRxMtuSize(23); //If not set RX MTU size, default is: 23 bytes.

	//set scan parameter and scan enable
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_50MS, SCAN_INTERVAL_50MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

	#if(FEATURE_PM_ENABLE)
		//
	#else
		bls_pm_setSuspendMask (SUSPEND_DISABLE);
	#endif
}



_attribute_ram_code_ void function_m2M_md_test_init_deepRetn(void)
{
#if (FEATURE_DEEPSLEEP_RETENTION_ENABLE)
	//
#endif
}



void function_m2M_md_test_mainloop(void)
{
	/////////////////////////////////////// HCI ///////////////////////////////////////
	blc_hci_proc ();

	////////////////////////////////////// UI entry /////////////////////////////////
	static u8 button_detect_en = 0;
	if(!button_detect_en && clock_time_exceed(0, 1000000)){// proc button 1 second later after power on
		button_detect_en = 1;
	}
	static u32 button_detect_tick = 0;
	if(button_detect_en && clock_time_exceed(button_detect_tick, 5000))
	{
		button_detect_tick = clock_time();
		proc_button();
	}

	if( host_update_conn_param_req && clock_time_exceed(host_update_conn_param_req, 50000))
	{
		host_update_conn_param_req = 0;
		if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){  //still in connection state
			blm_ll_updateConnection (cur_conn_handle, host_update_conn_min, host_update_conn_min, host_update_conn_latency,  host_update_conn_timeout, 0, 0 );
		}
	}

	if(connect_event_occurTick && clock_time_exceed(connect_event_occurTick, 500000)){  //500ms after connection established
		connect_event_occurTick = 0;

		printf("After conn 500ms, if not receive S's PHY exchange pkt, M send PHY exchange req to S.\n");
		blc_ll_setPhy(BLM_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_2M, 	 PHY_PREFER_2M,    CODED_PHY_PREFER_NONE);
	}

	//terminate and unpair proc
	static int master_disconnect_flag;
	if(dongle_unpair_enable){
		if(!master_disconnect_flag && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			if( blm_ll_disconnect(cur_conn_handle, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){
				master_disconnect_flag = 1;
				dongle_unpair_enable = 0;

				#if (BLE_HOST_SMP_ENABLE)
					int device_is_bond = tbl_bond_slave_search(cur_conn_mac_adrType, cur_conn_mac_addr);
					if(device_is_bond){ //this adv mac is bonded in master
						tbl_bond_slave_delete_by_adr(cur_conn_mac_adrType, cur_conn_mac_addr);  //by telink stack host smp
					}
				#endif
			}
		}
	}
	if(master_disconnect_flag && blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		master_disconnect_flag = 0;
	}

	if(start_2m_pkt_write_pendingTick && clock_time_exceed(start_2m_pkt_write_pendingTick, 1000000)){ // >1s
		if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && start_2m_pkt_write_test && !blm_ll_isRfStateMachineBusy())
		{
			if(BLE_SUCCESS == blc_gatt_pushWriteComand(BLM_CONN_HANDLE, SPP_CLIENT_TO_SERVER_DP_H, test_data, 20)){  //current data push TX fifo OK
				seqNoSend++;
				*(u32*)test_data = seqNoSend;
			}
		}
	}

}


#endif
