/********************************************************************************************************
 * @file     blm_host.c
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
#include "stack/ble/attr/att.h"
#include "stack/ble/ll/ll_conn/ll_conn.h"
#include "stack/ble/ll/ll_conn/ll_slave.h"
#include "stack/ble/ll/ll_conn/ll_master.h"

#include "blms_att.h"
#include "blms_pair.h"
#include "blms_host.h"
#include "blms_ota.h"



main_service_t		main_service = 0;

#define SMP_PENDING					1   //security management
#define SDP_PENDING					2   //service discovery

int	app_host_smp_sdp_pending = 0; 		//security & service discovery
int app_host_need_sdp_flg    = 0;

dev_char_info_t cur_conn_device;



////////////////////////////////////////////////////////////////////////////////////
// 							LED Handle
////////////////////////////////////////////////////////////////////////////////////
void app_led_en (int id, int en)
{
	//printf("\nled:0x%x %s\n", id, en ? "ON" : "OFF");

#if (UI_LED_ENABLE)
	en = !(LED_ON_LEVAL ^ en);
	if (id == (BLS_CONN_HANDLE | 0))
	{
		gpio_write(GPIO_LED_GREEN, en);
	}
	else if (id == (BLM_CONN_HANDLE | 1))
	{
		gpio_write(GPIO_LED_RED, en);
	}
	else if (id == (BLM_CONN_HANDLE | 2))
	{
		gpio_write(GPIO_LED_YELLOW, en);
	}
	else if (id == (BLM_CONN_HANDLE | 3))
	{
		gpio_write(GPIO_LED_BLUE, en);
	}
#endif
}



int master_auto_connect = 0;
int user_manual_paring;



const u8 	telink_adv_trigger_paring[] = {5, 0xFF, 0x11, 0x02, 0x01, 0x00};
const u8 	telink_adv_trigger_unpair[] = {5, 0xFF, 0x11, 0x02, 0x01, 0x01};

const u8 	telink_adv_trigger_paring_8258[] = {7, 0xFF, 0x11, 0x02, 0x01, 0x00, 0x58, 0x82};
const u8 	telink_adv_trigger_unpair_8258[] = {7, 0xFF, 0x11, 0x02, 0x01, 0x01, 0x58, 0x82};


#if (BLE_HOST_SIMPLE_SDP_ENABLE)
	extern void host_att_service_disccovery_clear(void);
	int host_att_client_handler (u16 connHandle, u8 *p);
	ble_sts_t  host_att_discoveryService (u16 handle, att_db_uuid16_t *p16, int n16, att_db_uuid128_t *p128, int n128);


	#define		ATT_DB_UUID16_NUM		20
	#define		ATT_DB_UUID128_NUM		8


	extern const u8 my_MicUUID[16];
	extern const u8 my_SpeakerUUID[16];
	extern const u8 my_OtaUUID[16];


	void app_service_discovery ()
	{
		att_db_uuid16_t 	db16[ATT_DB_UUID16_NUM];
		att_db_uuid128_t 	db128[ATT_DB_UUID128_NUM];
		memset (db16, 0, ATT_DB_UUID16_NUM * sizeof (att_db_uuid16_t));
		memset (db128, 0, ATT_DB_UUID128_NUM * sizeof (att_db_uuid128_t));

		if ( blms_is_handle_valid(cur_conn_device.conn_handle) && \
			 host_att_discoveryService (cur_conn_device.conn_handle, db16, ATT_DB_UUID16_NUM, db128, ATT_DB_UUID128_NUM) == BLE_SUCCESS)	// service discovery OK
		{
			cur_conn_device.char_handle[0] = blm_att_findHandleOfUuid128 (db128, my_MicUUID);			//MIC
			cur_conn_device.char_handle[1] = blm_att_findHandleOfUuid128 (db128, my_SpeakerUUID);		//Speaker
			cur_conn_device.char_handle[2] = blm_att_findHandleOfUuid128 (db128, my_OtaUUID);			//OTA

			cur_conn_device.char_handle[3] = blm_att_findHandleOfUuid16 (db16, CHARACTERISTIC_UUID_HID_REPORT,
						HID_REPORT_ID_CONSUME_CONTROL_INPUT | (HID_REPORT_TYPE_INPUT<<8));		//consume report

			cur_conn_device.char_handle[4] = blm_att_findHandleOfUuid16 (db16, CHARACTERISTIC_UUID_HID_REPORT,
						HID_REPORT_ID_KEYBOARD_INPUT | (HID_REPORT_TYPE_INPUT<<8));				//normal key report

			//module
			//cur_conn_device.char_handle[5] = blm_att_findHandleOfUuid128 (db128, my_SppS2CUUID);		//notify
			//cur_conn_device.char_handle[6] = blm_att_findHandleOfUuid128 (db128, my_SppC2SUUID);		//write_cmd

			//add the conn device to conn_dev_list after service discovery is correctly finished
			dev_char_info_insert(&cur_conn_device);

			printf("hdl[0]~hdl[4]:0x%x,0x%x,0x%x,0x%x,0x%x\n", cur_conn_device.char_handle[0],cur_conn_device.char_handle[1],cur_conn_device.char_handle[2],cur_conn_device.char_handle[3],cur_conn_device.char_handle[4]);

		}

		app_host_smp_sdp_pending = 0;  //service discovery finish
		//printf("[0x%x]SD finish\n", cur_conn_device.conn_handle);
	}

	void app_register_service (void *p)
	{
		main_service = p;
	}

#endif







#if (BLE_HOST_SMP_ENABLE)
int app_host_smp_finish (void)  //smp finish callback
{
	#if (BLE_HOST_SIMPLE_SDP_ENABLE)  //smp finish, start sdp
		if(app_host_smp_sdp_pending == SMP_PENDING)
		{
			if(app_host_need_sdp_flg){
				app_host_smp_sdp_pending = SDP_PENDING;  //service discovery busy
				app_register_service(&app_service_discovery);
				printf("[0x%x]SD register\n", cur_conn_device.conn_handle);
			}
			else{
				app_host_smp_sdp_pending = 0;  //no sdp
				printf("[0x%x]no need sdp\n", cur_conn_device.conn_handle);
			}
		}
	#else
		app_host_smp_sdp_pending = 0;  //no sdp
	#endif

	return 0;
}
#endif







int blm_le_adv_report_event_handle(u8 *p)
{
	event_adv_report_t *pa = (event_adv_report_t *)p;
	s8 rssi = pa->data[pa->len];

	 //if previous connection smp&sdp not finish, can not create a new connection
	if(app_host_smp_sdp_pending){
		return 1;
	}




#if 0  //debug if ADV report OK
	u8 locate_mac[6] = {0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
	if(!memcmp(locate_mac,  pa->mac, 6)){

	}
#endif


	/****************** Button press or Adv pair packet triggers pair ***********************/
	int master_auto_connect = 0;
	int user_manual_paring = 0;

	//manual paring methods 1: button triggers
	user_manual_paring = dongle_pairing_enable && (rssi > -56);  //button trigger pairing(rssi threshold, short distance)

	//manual paring methods 2: special paring adv data
	if(!user_manual_paring){  //special adv pair data can also trigger pairing
		user_manual_paring = (memcmp(pa->data, telink_adv_trigger_paring_8258, sizeof(telink_adv_trigger_paring_8258)) == 0) && (rssi > -56);
	}


	#if (BLE_HOST_SMP_ENABLE)
		master_auto_connect = blms_smp_searchBondingDevice_in_Flash_by_Address(pa->adr_type, pa->mac);
	#else
		//search in slave mac table to find whether this device is an old device which has already paired with master
		master_auto_connect = 0; // user_tbl_slave_mac_search(pa->adr_type, pa->mac);
	#endif

	if(master_auto_connect || user_manual_paring)
	{


		//send create connection cmd to controller, trigger it switch to initiating state, after this cmd,
		//controller will scan all the adv packets it received but not report to host, to find the specified
		//device(adr_type & mac), then send a connection request packet after 150us, enter to connection state
		// and send a connection complete event(HCI_SUB_EVT_LE_CONNECTION_COMPLETE)
		u8 status = blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, INITIATE_FP_ADV_SPECIFY,  \
								 pa->adr_type, pa->mac, BLE_ADDR_PUBLIC, \
								 CONN_INTERVAL_38P75MS, CONN_INTERVAL_38P75MS, 0, CONN_TIMEOUT_4S, \
								 0, 0xFFFF);

#if 0
		if(status == BLE_SUCCESS)   //create connection success
		{
			#if (!BLE_HOST_SMP_ENABLE)
				if(user_manual_paring && !master_auto_connect){  //manual pair
					blm_manPair.manual_pair = 1;
					blm_manPair.mac_type = pa->adr_type;
					memcpy(blm_manPair.mac, pa->mac, 6);
					blm_manPair.pair_tick = clock_time();
				}
			#endif
		}
#endif

	}



#if 0
	/****************** Adv unpair packet triggers unpair ***********************/
	int adv_unpair_en = !memcmp(pa->data, telink_adv_trigger_unpair_8258, sizeof(telink_adv_trigger_unpair_8258));
	if(adv_unpair_en)
	{
		int device_is_bond;

		#if (BLE_HOST_SMP_ENABLE)
			device_is_bond = tbl_bond_slave_search(pa->adr_type, pa->mac);
			if(device_is_bond){ //this adv mac is bonded in master
				tbl_bond_slave_delete_by_adr(pa->adr_type, pa->mac);  //by telink stack host smp
			}
		#else
			device_is_bond = user_tbl_slave_mac_search(pa->adr_type, pa->mac);
			if(device_is_bond){ //this adv mac is bonded in master
				user_tbl_slave_mac_delete_by_adr(pa->adr_type, pa->mac);  //by user application code
			}
		#endif
	}

#endif


	return 0;
}





int blm_le_connection_establish_event_handle(u8 *p)
{
	event_connection_complete_t *pCon = (event_connection_complete_t *)p;

	printf("*** [0x%x]connection establish: 0x%x ***\n", pCon->handle, pCon->status);

	if(pCon->status == BLE_SUCCESS){

		app_led_en (pCon->handle, 1);


		u8 idx = pCon->handle & (BLMS_MAX_CONN_NUM-1);
		st_llms_conn_t* pc = (st_llms_conn_t*)&blms[idx];

		if(1 && blms_getCurrConnRole(pc->connHandle) == LL_ROLE_MASTER) //att client handler
		{
			memset(&cur_conn_device, 0, sizeof(dev_char_info_t));

			//save current connect address type and address and conn_handle
			cur_conn_device.conn_handle = pCon->handle;
			cur_conn_device.mac_adrType = pCon->peer_adr_type;
			memcpy(cur_conn_device.mac_addr, pCon->mac, 6);

			#if (BLE_HOST_SMP_ENABLE)
				app_host_smp_sdp_pending = SMP_PENDING; //pair & security first
			#endif

			app_host_need_sdp_flg = 1;
			u8 status = dec_char_info_fix_state_connhandle_by_mac (1, pCon->handle, pCon->peer_adr_type, pCon->mac);

			if(status){
				//not need do sdp
				app_host_need_sdp_flg = 0;
			}
		}
	}

	return 0;
}





int 	blm_disconnect_event_handle(u8 *p)
{
	event_disconnection_t	*pd = (event_disconnection_t *)p;

	printf("*** [0x%x]terminate reason: 0x%x ***\n", pd->handle, pd->reason);

	//terminate reason
	//connection timeout
	if(pd->reason == HCI_ERR_CONN_TIMEOUT){

	}
	//peer device(slave) send terminate cmd on link layer
	else if(pd->reason == HCI_ERR_REMOTE_USER_TERM_CONN){

	}
	//master host disconnect( blm_ll_disconnect(current_connHandle, HCI_ERR_REMOTE_USER_TERM_CONN) )
	else if(pd->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){

	}
	 //master create connection, send conn_req, but did not received acked packet in 6 connection event
	else if(pd->reason == HCI_ERR_CONN_FAILED_TO_ESTABLISH){
		//when controller is in initiating state, find the specified device, send connection request to slave,
		//but slave lost this rf packet, there will no ack packet from slave, after 6 connection events, master
		//controller send a disconnect event with reason HCI_ERR_CONN_FAILED_TO_ESTABLISH
		//if slave got the connection request packet and send ack within 6 connection events, controller
		//send connection establish event to host(telink defined event)

	}
	else{

	}

	app_led_en (pd->handle, 0);

	//if previous connection smp&sdp not finished, clear this flag
	if(app_host_smp_sdp_pending){
		app_host_smp_sdp_pending = 0;
	}


	dec_char_info_fix_state_by_connHandle (0, pd->handle);


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
			blm_disconnect_event_handle(p);
		}

		#if (BLE_HOST_SMP_ENABLE)
			else if(evtCode == HCI_EVT_ENCRYPTION_CHANGE)
			{
				event_enc_change_t *pe = (event_enc_change_t *)p;
				blms_smp_encChangeEvt(pe->status, pe->handle, pe->enc_enable);
			}
			else if(evtCode == BLM_CONN_ENC_REFRESH)
			{
				event_enc_refresh_t *pe = (event_enc_refresh_t *)p;
				blms_smp_encChangeEvt(pe->status, pe->handle, 1);
			}
		#endif

		else if(evtCode == HCI_EVT_LE_META)
		{
			u8 subEvt_code = p[0];


			//------hci le event: le connection complete event---------------------------------
			if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
			{
				//after controller is set to initiating state by host (blc_ll_createConnection(...) )
				//it will scan the specified device(adr_type & mac), when find this adv packet, send a connection request packet to slave
				//and enter to connection state, send connection complete event. but notice that connection complete not
				//equals to connection establish. connection complete means that master controller set all the ble timing
				//get ready, but has not received any slave packet, if slave rf lost the connection request packet, it will
				//not send any packet to master controller
			}
			//------hci le event: le connection establish event---------------------------------
			else if(subEvt_code == HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)  //connection establish(telink private event)
			{
				//notice that: this connection event is defined by telink, not a standard ble controller event
				//after master controller send connection request packet to slave, when slave received this packet
				//and enter to connection state, send a ack packet within 6 connection event, master will send
				//connection establish event to host(HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)

				blm_le_connection_establish_event_handle(p);
			}
			//--------hci le event: le adv report event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
			{
				//after controller is set to scan state, it will report all the adv packet it received by this event

				blm_le_adv_report_event_handle(p);
			}
			//------hci le event: le connection update complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)	// connection update
			{
				//after master host send update conn param req cmd to controller( blm_ll_updateConnection(...) ),
				//when update take effect, controller send update complete event to host
			}
		}
	}


	return 0;

}


int app_host_event_callback (u32 h, u8 *para, int n)
{
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PARING_BEAGIN:
		{
			gap_smp_paringBeginEvt_t *p = (gap_smp_paringBeginEvt_t *)para;
			/*
			 * 	JustWorks = 0, PK_Init_Dsply_Resp_Input = 1, PK_Resp_Dsply_Init_Input = 2, PK_BOTH_INPUT = 3,OOB_Authentication = 4, Numric_Comparison = 5,
			 */
			u8 TK_method[6][40] = {"JustWorks", "PK_Init_Dsply_Resp_Input", "PK_Resp_Dsply_Init_Input", "PK_BOTH_INPUT", "OOB_Authentication", "Numric_Comparison"};
			printf("[0x%x]Pairing begin (sc:%s,TK-method:%s)\n", p->connHandle, p->secure_conn?"SC":"Legacy", TK_method[p->tk_method]);
			//printf("[0x%x]Pairing begin\n", p->connHandle);
		}
		break;

		case GAP_EVT_SMP_PARING_SUCCESS:
		{
			gap_smp_paringSuccessEvt_t* p = (gap_smp_paringSuccessEvt_t*)para;
			printf("[0x%x]Pairing success: bond%s (save %s)\n", p->connHandle, p->bonding ?"T":"F", p->bonding_result ? "OK" : "ERR");
		}
		break;

		case GAP_EVT_SMP_PARING_FAIL:
		{
			gap_smp_paringFailEvt_t* p = (gap_smp_paringFailEvt_t*)para;
			printf("[0x%x]Pairing failed:rsn:0x%x\n", p->connHandle, p->reason);
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
			printf("[0x%x]ENC done (%s)\n", p->connHandle, p->re_connect == SMP_STANDARD_PAIR ? "1stParing" :"reConnect");
		}
		break;

		case GAP_EVT_SMP_TK_DISPALY:
		{
			char pc[7];
			gap_smp_TkDisplayEvt_t* p = (gap_smp_TkDisplayEvt_t*)para;
			sprintf(pc, "%d", p->tk_pincode);
			printf("[0x%x]TK display:%s\n", p->connHandle, pc);
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
		{
			gap_smp_TkReqPassKeyEvt_t* p = (gap_smp_TkReqPassKeyEvt_t*)para;
			printf("[0x%x]TK Request passkey\n", p->connHandle);
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_OOB:
		{

		}
		break;

		case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
		{

		}
		break;

		case GAP_EVT_ATT_EXCHANGE_MTU:
		{

		}
		break;

		case GAP_EVT_GATT_HANDLE_VLAUE_CONFIRM:
		{

		}
		break;

		default:
		break;
	}

	return 0;
}



int app_l2cap_handler (u16 connHandle, u8 *raw_pkt)
{
	//l2cap data packeted, make sure that user see complete l2cap data
	u8* pkt = blms_l2cap_packet_pack (connHandle, raw_pkt);
	rf_packet_l2cap_t *ptrL2cap = (rf_packet_l2cap_t*)(pkt+DMA_RFRX_OFFSET_HEADER);

	//printf("\n\r==========================================================>\n");
	//printf("L2CAP pkt:");array_printf((u8*)ptrL2cap, ptrL2cap->rf_len+2);
	//printf("<==========================================================\n\r");

	if (ptrL2cap){
		//l2cap data channel id, 4 for ATT, 5 for Signal, 6 for SMP
		if(1 && ptrL2cap->chanId == L2CAP_CID_ATTR_PROTOCOL)  //att data
		{
			//printf("ATT:0x%x\n", connHandle);

			if(blms_getCurrConnRole(connHandle) == LL_ROLE_SLAVE){
				//ATT
				u8 opcode = ptrL2cap->opcode;
				if( !(opcode & 0x01) || opcode == ATT_OP_EXCHANGE_MTU_RSP)		//att server handler
				{
					u8 *pr = l2cap_att_handler (connHandle, pkt);
					if (pr)
					{
						//printf("s->m: ATT RSP");array_printf(pr+4, pr[5]+2);printf("\n\r");
						blms_ll_pushTxfifo_hold (connHandle, pr + DMA_RFRX_OFFSET_HEADER); //remove ATT hold, send insufficient security
					}
				}
			}
			else if(1 && blms_getCurrConnRole(connHandle) == LL_ROLE_MASTER) //att client handler
			{
				#if (BLE_HOST_SIMPLE_SDP_ENABLE)
					if(app_host_smp_sdp_pending == SDP_PENDING)  //ATT service discovery is ongoing
					{
						//when service discovery function is running, all the ATT data from slave
						//will be processed by it,  user can only send your own att cmd after  service discovery is over
						host_att_client_handler (connHandle, (u8 *)ptrL2cap); //handle this ATT data by service discovery process
					}
				#endif

				//so any ATT data before service discovery will be dropped
				dev_char_info_t* dev_info = dev_char_info_search_by_connhandle (connHandle);
				if(dev_info)
				{
					//-------	user process ------------------------------------------------
					rf_packet_att_t *pAtt = (rf_packet_att_t*)ptrL2cap;
					u16 attHandle = pAtt->handle0 | pAtt->handle1<<8;

					if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //slave handle notify
					{
						st_llms_conn_t *pc = &blms[0];
						u16 slaveConnHandle = 0;

						if(pc->connState){
							slaveConnHandle = pc->connHandle;
						}

						//---------------	consumer key --------------------------
						if(attHandle == dev_info->char_handle[3])
						{
							att_keyboard_media (connHandle, pAtt->dat);

							if(slaveConnHandle){
								u16 consumer_key = pAtt->dat[0] | pAtt->dat[1]<<8;
								blms_att_pushNotifyData (slaveConnHandle, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
							}

							printf("[0x%x]CK[:0x%x]\n", connHandle, pAtt->dat[0] | pAtt->dat[1]<<8);
						}
						//---------------	keyboard key --------------------------
						else if(attHandle == dev_info->char_handle[4])
						{
							att_keyboard (connHandle, pAtt->dat);

							if(slaveConnHandle){
								u8 key_buf[8];
								key_buf[2] = pAtt->dat[2];
								blms_att_pushNotifyData (slaveConnHandle, HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
							}

							printf("[0x%x]KK[:0x%x]\n", connHandle, pAtt->dat[2]);
						}
						else
						{

						}
					}
					else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_IND)
					{

					}
				}

			}
		}
		else if(ptrL2cap->chanId == L2CAP_CID_SIG_CHANNEL)  //signal
		{
			//printf("SIG:0x%x\n", connHandle);

			if(ptrL2cap->opcode == L2CAP_CMD_CONN_UPD_PARA_REQ)  //slave send conn param update req on l2cap
			{
				rf_packet_l2cap_connParaUpReq_t *req = (rf_packet_l2cap_connParaUpReq_t *)ptrL2cap;
				u8 conn_update_rsp[16];  //12 + 4(mic)
				rf_packet_l2cap_connParaUpRsp_t *pRsp = (rf_packet_l2cap_connParaUpReq_t *)conn_update_rsp;
				pRsp->llid = L2CAP_FIRST_PKT_C2H;
				pRsp->rf_len = 10;
				pRsp->l2capLen = 6;
				pRsp->chanId = L2CAP_CID_SIG_CHANNEL;
				pRsp->opcode = 0x13;
				pRsp->id = req->id;
				pRsp->data_len = 2;
				pRsp->result = CONN_PARAM_UPDATE_REJECT;
				blms_ll_pushTxfifo (connHandle, conn_update_rsp);//send SIG Connection Param Update Response
			}
		}
		else if(1 && ptrL2cap->chanId == L2CAP_CID_SMP) //smp
		{
			//printf("SMP:0x%x\n", connHandle);

			u8* pr = l2cap_ms_smp_handler(connHandle, (u8*)(pkt+DMA_RFRX_OFFSET_HEADER));

			if (pr){
				blms_ll_pushTxfifo (connHandle, pr); //remove ATT hold, send insufficient security
			}
		}
		else
		{
			printf("L2CAP err:0x%x\n", connHandle);
		}
	}

	return 0;
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEVICE_CHAR_INFO_MAX_NUM		6

/*
 * Used for store information of connected devices.
 */
dev_char_info_t conn_dev_list[DEVICE_CHAR_INFO_MAX_NUM]; //6 num
const u8 ZERO10[10] = {0};

/*
 * Used for add device information to conn_dev_list.
 */
int dev_char_info_insert (dev_char_info_t* dev_char_info)
{
	s8 idle = -1;

	// if no idle buffer, to find a no connect state buffer.
	if(idle == -1)
	{
		foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
		{
			if(0 == conn_dev_list[i].conn_state)
			{
				idle = i;
				//printf("fncb:%d\n", i);
				break;
			}
		}
	}

	//-- find an idle buffer to store data.
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(0 == memcmp (ZERO10, conn_dev_list[i].char_handle, CHAR_HANDLE_MAX))
		{
			idle = i;
			//printf("fib:%d\n", i);
			break;
		}
	}

	if(idle == -1){
		printf("nb\n");
		return 0;
	}

	//printf("insert[idle:%d][handle:0x%x]\n", idle, dev_char_info->conn_handle);

	memcpy(&conn_dev_list[idle], dev_char_info, sizeof(dev_char_info_t));
	conn_dev_list[idle].conn_state = 1;

	return 1;
}

/*
 * Used for delete device information from conn_dev_list.
 */
int dev_char_info_delete_by_mac (u8 adr_type, u8* mac_addr)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(adr_type == conn_dev_list[i].mac_adrType && (!memcmp (mac_addr, conn_dev_list[i].mac_addr, 6)) )
		{
			memset(&conn_dev_list[i], 0, sizeof(dev_char_info_t));
			return 0;
		}
	}

	return 1;  //no find the peer device address.
}


/*
 * Used for delete device information from conn_dev_list.
 */
int dev_char_info_delete_by_connhandle (u16 connhandle)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(conn_dev_list[i].conn_handle == connhandle && conn_dev_list[i].conn_state)
		{
			memset(&conn_dev_list[i], 0, sizeof(dev_char_info_t));
			//printf("clr[idle:%d][handle:0x%x]\n", i, connhandle);
			return 0;
		}
	}

	return 1;  //no find the peer device address.
}

/*
 * Used for search device information from conn_dev_list.
 */
dev_char_info_t* dev_char_info_search_by_mac (u8 adr_type, u8* mac_addr)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(adr_type == conn_dev_list[i].mac_adrType && (!memcmp (mac_addr, conn_dev_list[i].mac_addr, 6)) )
		{
			return &conn_dev_list[i];   // find the peer device
		}
	}

	return 0;  // no find the peer device
}

/*
 * Used for search device information from conn_dev_list by connhandle.
 * return  - 0 no peer device.
 * 			 others - device info
 */
dev_char_info_t* dev_char_info_search_by_connhandle (u16 connhandle)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(conn_dev_list[i].conn_handle == connhandle && conn_dev_list[i].conn_state)
		{
			//printf("[0x%x]find the peer device[i:%d]\n", connhandle, i);
			return &conn_dev_list[i];   // find the peer device
		}
	}
	//printf("[0x%x]no find the peer device\n", connhandle);
	return 0;  // no find the peer device
}


/*
 * Used for fix device state in conn_dev_list via device mac addr.
 */
dev_char_info_t* dec_char_info_fix_state (u8 adr_type, u8* mac_addr, u8 state, u16 conn_handle)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(adr_type == conn_dev_list[i].mac_adrType && (!memcmp (mac_addr, conn_dev_list[i].mac_addr, 6)) )
		{
			conn_dev_list[i].conn_state = state;
			conn_dev_list[i].conn_handle = conn_handle;
			return &conn_dev_list[i];   // find the peer device
		}
	}

	return 0;  // no find the peer device
}

int dec_char_info_fix_state_by_connHandle (u8 state, u8 conn_handle)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(conn_dev_list[i].conn_handle == conn_handle)
		{
			conn_dev_list[i].conn_state = state;
			//printf("remove[idle:%d][handle:0x%x]\n", i, conn_handle);
		}
	}

	return 1;  // no find the peer device
}

int dec_char_info_fix_state_connhandle_by_mac (u8 state, u8 conn_handle, u8 adr_type, u8* mac_addr)
{
	foreach (i, DEVICE_CHAR_INFO_MAX_NUM)
	{
		if(adr_type == conn_dev_list[i].mac_adrType && (!memcmp (mac_addr, conn_dev_list[i].mac_addr, 6)) )
		{
			conn_dev_list[i].conn_state = state;
			conn_dev_list[i].conn_handle = conn_handle;
			//printf("chg[idle:%d][handle:0x%x]\n", i, conn_handle);
			return 1;  // find the peer device
		}
	}
	//printf("[%d]no find the peer device\n", conn_handle);
	return 0;  // no find the peer device
}
