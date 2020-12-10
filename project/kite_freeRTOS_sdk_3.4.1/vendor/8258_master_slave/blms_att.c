/********************************************************************************************************
 * @file  	 blm_att.c
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
#include "stack/ble/smpc/smp_ms_storage.h"

#include "blms_att.h"
#include "blms_pair.h"
#include "blms_host.h"
#include "blms_ota.h"

#include "application/keyboard/keyboard.h"
#include "application/rf_frame.h"




////////////////////////////////////////// master-role ATT concerned ///////////////////////////////////////////////
#define     TELINK_UNPAIR_KEYVALUE		0xFF  //conn state, unpair


const u8 my_MicUUID[16]			= TELINK_MIC_DATA;
const u8 my_SpeakerUUID[16]		= TELINK_SPEAKER_DATA;
const u8 my_OtaUUID[16]			= TELINK_SPP_DATA_OTA;
const u8 my_SppS2CUUID[16]		= TELINK_SPP_DATA_SERVER2CLIENT;
const u8 my_SppC2SUUID[16]		= TELINK_SPP_DATA_CLIENT2SERVER;



u8 read_by_type_req_uuid[16] = {};
u8 read_by_type_req_uuidLen;

u16 	current_read_req_handle;

void host_att_set_current_readByTypeReq_uuid(u8 *uuid, u8 uuid_len)
{
	read_by_type_req_uuidLen = uuid_len;
	memcpy(read_by_type_req_uuid, uuid, uuid_len);
}







u8	*p_att_response = 0;

volatile u32	host_att_req_busy = 0;

int host_att_client_handler (u16 connHandle, u8 *p)
{
	att_readByTypeRsp_t *p_rsp = (att_readByTypeRsp_t *) p;
	if (p_att_response)
	{
		if ((connHandle & 7) == (host_att_req_busy & 7) && p_rsp->chanId == 0x04 &&
				(p_rsp->opcode == 0x01 || p_rsp->opcode == ((host_att_req_busy >> 16) | 1)))
		{
			memcpy (p_att_response, p, 32);
			host_att_req_busy = 0;
		}
	}
	return 0;
}

void host_att_service_disccovery_clear(void)
{
	p_att_response = 0;
}

typedef int (*host_att_idle_func_t) (void);
host_att_idle_func_t host_att_idle_func = 0;

int host_att_register_idle_func (void *p)
{
	if (host_att_idle_func)
		return 1;

	host_att_idle_func = p;
	return 0;
}

int host_att_response ()
{
	return host_att_req_busy == 0;
}


int host_att_service_wait_event (u16 handle, u8 *p, u32 timeout)
{
	host_att_req_busy = handle | (p[6] << 16);
	p_att_response = p;
	blms_ll_pushTxfifo(handle, p);

	u32 t = clock_time ();
	while (!clock_time_exceed (t, timeout))
	{
		if (host_att_response ())
		{
			return 0;
		}
		if (host_att_idle_func)
		{
			if (host_att_idle_func ())
			{
				break;
			}
		}
	}
	return 1;
}


ble_sts_t  host_att_discoveryService (u16 handle, att_db_uuid16_t *p16, int n16, att_db_uuid128_t *p128, int n128)
{
	att_db_uuid16_t *ps16 = p16;
	att_db_uuid128_t *ps128 = p128;
	int i16 = 0;
	int i128 = 0;

	ps16->num = 0;
	ps128->num = 0;

	u8 idx = handle & (BLMS_MAX_CONN_NUM-1);
	st_llms_conn_t* pc = (st_llms_conn_t*)&blms[idx];

	// char discovery: att_read_by_type
		// hid discovery: att_find_info
	u8  dat[32];
	u16 s = 1;
	u16 uuid = GATT_UUID_CHARACTER;
	do {

		att_req_read_by_type (dat, s, 0xffff, (u8 *)&uuid, 2);
		if (host_att_service_wait_event(handle, dat, 1000000)) // 1s
		{
			return  GATT_ERR_SERVICE_DISCOVERY_TIEMOUT;			//timeout
		}

		// process response data
		att_readByTypeRsp_t *p_rsp = (att_readByTypeRsp_t *) dat;
		if (p_rsp->opcode != ATT_OP_READ_BY_TYPE_RSP)
		{
			break;
		}

		if (p_rsp->datalen == 21)		//uuid128
		{
			s = p_rsp->data[3] + p_rsp->data[4] * 256;
			if (i128 < n128)
			{
				p128->property = p_rsp->data[2];
				p128->handle = s;
				memcpy (p128->uuid, p_rsp->data + 5, 16);
				i128++;
				p128++;
			}
		}
		else if (p_rsp->datalen == 7) //uuid16
		{
			u8 *pd = p_rsp->data;
			while (p_rsp->l2capLen > 7)
			{
				s = pd[3] + pd[4] * 256;
				if (i16 < n16)
				{
					p16->property = pd[2];
					p16->handle = s;
					p16->uuid = pd[5] | (pd[6] << 8);
					p16->ref = 0;
					i16 ++;
					p16++;
				}
				p_rsp->l2capLen -= 7;
				pd += 7;
			}
		}
	} while (1);

	ps16->num = i16;
	ps128->num = i128;

	//--------- use att_find_info to find the reference property for hid ----------
	p16 = ps16;
	for (int i=0; i<i16; i++)
	{
		if (p16->uuid == CHARACTERISTIC_UUID_HID_REPORT)		//find reference
		{

			att_req_find_info (dat, p16->handle, 0xffff);
			if (host_att_service_wait_event(handle, dat, 1000000)) // 1s
			{
				return  GATT_ERR_SERVICE_DISCOVERY_TIEMOUT;			//timeout
			}

			att_findInfoRsp_t *p_rsp = (att_findInfoRsp_t *) dat;
			if (p_rsp->opcode == ATT_OP_FIND_INFO_RSP && p_rsp->format == 1)
			{
				int n = p_rsp->l2capLen - 2;
				u8 *pd = p_rsp->data;
				while (n > 0)
				{
					if ((pd[2]==U16_LO(GATT_UUID_CHARACTER) && pd[3]==U16_HI(GATT_UUID_CHARACTER)) ||
						(pd[2]==U16_LO(GATT_UUID_PRIMARY_SERVICE) && pd[3]==U16_HI(GATT_UUID_PRIMARY_SERVICE))	)
					{
						break;
					}

					if (pd[2]==U16_LO(GATT_UUID_REPORT_REF) && pd[3]==U16_HI(GATT_UUID_REPORT_REF))
					{
					//-----------		read attribute ----------------

						att_req_read (dat, pd[0]);
						if (host_att_service_wait_event(handle, dat, 1000000))
						{
							return  GATT_ERR_SERVICE_DISCOVERY_TIEMOUT;			//timeout
						}

						att_readRsp_t *pr = (att_readRsp_t *) dat;
						if (pr->opcode == ATT_OP_READ_RSP)
						{
							p16->ref = pr->value[0] | (pr->value[1] << 8);
						}

						break;
					}
					n -= 4;
					pd += 4;
				}
			}
		} //----- end for if CHARACTERISTIC_UUID_HID_REPORT

		p16++;
	}

	return  BLE_SUCCESS;
}







rf_packet_mouse_t	pkt_mouse = {
		sizeof (rf_packet_mouse_t) - 4,	// dma_len

		sizeof (rf_packet_mouse_t) - 5,	// rf_len
		RF_PROTO_BYTE,		// proto
		PKT_FLOW_DIR,		// flow
		FRAME_TYPE_MOUSE,					// type

//		U32_MAX,			// gid0

		0,					// rssi
		0,					// per
		0,					// seq_no
		1,					// number of frame
};


extern void usbmouse_add_frame (rf_packet_mouse_t *packet_mouse);

void	att_mouse (u16 conn, u8 *p)
{
	memcpy (pkt_mouse.data, p, 4);
	pkt_mouse.seq_no++;
    usbmouse_add_frame(&pkt_mouse);
}



extern void usbkb_hid_report(kb_data_t *data);
extern void report_to_KeySimTool(u8 len,u8 * keycode);
extern void usbkb_report_consumer_key(u16 consumer_key);

extern void report_media_key_to_KeySimTool(u16);

void	att_keyboard_media (u16 conn, u8 *p)
{
	u16 media_key = p[0] | p[1]<<8;


#if (UI_UPPER_COMPUTER_ENABLE)
	if(read_reg8(0) == 0x5a){ //report to KeySimTool  mode
		report_media_key_to_KeySimTool(media_key);
	}
	else{
		usbkb_report_consumer_key(media_key);
	}
#else
	usbkb_report_consumer_key(media_key);
#endif

}

//////////////// keyboard ///////////////////////////////////////////////////
int Adbg_att_kb_cnt = 0;
kb_data_t		kb_dat_report = {1, 0, {0,0,0,0,0,0} };
int keyboard_not_release = 0;
extern int 	dongle_unpair_enable;
void	att_keyboard (u16 conn, u8 *p)
{
	Adbg_att_kb_cnt ++;

	memcpy(&kb_dat_report, p, sizeof(kb_data_t));

	if(kb_dat_report.keycode[0] == TELINK_UNPAIR_KEYVALUE){ //slave special unpair cmd

		if(!dongle_unpair_enable){
			dongle_unpair_enable = 1;
		}

		return;  //TELINK_UNPAIR_KEYVALUE not report
	}


	if (kb_dat_report.keycode[0])  			//keycode[0]
	{
		kb_dat_report.cnt = 1;  //1 key value
		keyboard_not_release = 1;
	}
	else{
		kb_dat_report.cnt = 0;  //key release
		keyboard_not_release = 0;
	}


#if (UI_UPPER_COMPUTER_ENABLE)
	if(read_reg8(0) == 0x5a){ //report to KeySimTool  mode
		report_to_KeySimTool(kb_dat_report.cnt, kb_dat_report.keycode);
	}
	else{
		usbkb_hid_report((kb_data_t *) &kb_dat_report);
	}
#else
	usbkb_hid_report((kb_data_t *) &kb_dat_report);
#endif

}




void att_keyboard_release(void)
{
	kb_dat_report.cnt = 0;  //key release
//	usbkb_hid_report((kb_data_t *) &kb_dat_report);
}



void host_att_data_clear(void)
{
	if(keyboard_not_release){
		keyboard_not_release = 0;
		att_keyboard_release();
	}
}



void app_setCurrentReadReq_attHandle(u16 handle)
{
	current_read_req_handle = handle;
}

u16 app_getCurrentReadReq_attHandle(void)
{
	return current_read_req_handle;
}





////////////////////////////////////////// slave-role ATT service concerned ///////////////////////////////////////////////
typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

static const u16 reportRefUUID = GATT_UUID_REPORT_REF;

static const u16 characterPresentFormatUUID = GATT_UUID_CHAR_PRESENT_FORMAT;

static const u16 userdesc_UUID	= GATT_UUID_CHAR_USER_DESC;

static const u16 serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

static const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;

static const u16 my_appearanceUIID = GATT_UUID_APPEARANCE;

static const u16 my_periConnParamUUID = GATT_UUID_PERI_CONN_PARAM;

static const u16 my_appearance = GAP_APPEARE_UNKNOWN;

static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;

static const gap_periConnectParams_t my_periConnParameters = {20, 40, 0, 1000};

static u16 serviceChangeVal[2] = {0};

static u8 serviceChangeCCC[2] = {0,0};

static const u8 my_devName[] = {'M','-','S','t','e','s','t'};

static const u8 my_PnPtrs [] = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};

//////////////////////// Battery /////////////////////////////////////////////////
static const u16 my_batServiceUUID        = SERVICE_UUID_BATTERY;
static const u16 my_batCharUUID       	  = CHARACTERISTIC_UUID_BATTERY_LEVEL;
static u8 batteryValueInCCC[2] = {0,0};
static u8 my_batVal[1] 	= {99};

//////////////////////// HID /////////////////////////////////////////////////////

static const u16 my_hidServiceUUID        = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;

static const u16 hidServiceUUID           = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;
static const u16 hidProtocolModeUUID      = CHARACTERISTIC_UUID_HID_PROTOCOL_MODE;
static const u16 hidReportUUID            = CHARACTERISTIC_UUID_HID_REPORT;
static const u16 hidReportMapUUID         = CHARACTERISTIC_UUID_HID_REPORT_MAP;
static const u16 hidbootKeyInReportUUID   = CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT;
static const u16 hidbootKeyOutReportUUID  = CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT;
static const u16 hidbootMouseInReportUUID = CHARACTERISTIC_UUID_HID_BOOT_MOUSE_INPUT;
static const u16 hidinformationUUID       = CHARACTERISTIC_UUID_HID_INFORMATION;
static const u16 hidCtrlPointUUID         = CHARACTERISTIC_UUID_HID_CONTROL_POINT;
static const u16 hidIncludeUUID           = GATT_UUID_INCLUDE;

static const u8 protocolMode 			  = DFLT_HID_PROTOCOL_MODE;

// Key in Report characteristic variables
static u8 reportKeyIn[8];
static u8 reportKeyInCCC[2] = {0,0};
// HID Report Reference characteristic descriptor, key input
static const u8 reportRefKeyIn[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_INPUT };

// Key out Report characteristic variables
static u8 reportKeyOut[1];
static const u8 reportRefKeyOut[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_OUTPUT };

// Consumer Control input Report
static u8 reportConsumerControlIn[2];
static u8 reportConsumerControlInCCC[2] = {0,0};
static const u8 reportRefConsumerControlIn[2] =
			 { HID_REPORT_ID_CONSUME_CONTROL_INPUT, HID_REPORT_TYPE_INPUT };

// Boot Keyboard Input Report
static u8 bootKeyInReport;
static u8 bootKeyInReportCCC[2] = {0,0};

// Boot Keyboard Output Report
static u8 bootKeyOutReport;

// HID Information characteristic
static const u8 hidInformation[] =
{
  U16_LO(0x0111), U16_HI(0x0111),             // bcdHID (USB HID version)
  0x00,                                       // bCountryCode
  0x01                                        // Flags
};

// HID Control Point characteristic
static u8 controlPoint;

// HID Report Map characteristic
// Keyboard report descriptor (using format for Boot interface descriptor)

static const u8 reportMap[] =
{
	//keyboard report in
	0x05, 0x01,     // Usage Pg (Generic Desktop)
	0x09, 0x06,     // Usage (Keyboard)
	0xA1, 0x01,     // Collection: (Application)
	0x85, HID_REPORT_ID_KEYBOARD_INPUT,     // Report Id (keyboard)
				  //
	0x05, 0x07,     // Usage Pg (Key Codes)
	0x19, 0xE0,     // Usage Min (224)  VK_CTRL:0xe0
	0x29, 0xE7,     // Usage Max (231)  VK_RWIN:0xe7
	0x15, 0x00,     // Log Min (0)
	0x25, 0x01,     // Log Max (1)
				  //
				  // Modifier byte
	0x75, 0x01,     // Report Size (1)   1 bit * 8
	0x95, 0x08,     // Report Count (8)
	0x81, 0x02,     // Input: (Data, Variable, Absolute)
				  //
				  // Reserved byte
	0x95, 0x01,     // Report Count (1)
	0x75, 0x08,     // Report Size (8)
	0x81, 0x01,     // Input: (static constant)

	//keyboard output
	//5 bit led ctrl: NumLock CapsLock ScrollLock Compose kana
	0x95, 0x05,    //Report Count (5)
	0x75, 0x01,    //Report Size (1)
	0x05, 0x08,    //Usage Pg (LEDs )
	0x19, 0x01,    //Usage Min
	0x29, 0x05,    //Usage Max
	0x91, 0x02,    //Output (Data, Variable, Absolute)
	//3 bit reserved
	0x95, 0x01,    //Report Count (1)
	0x75, 0x03,    //Report Size (3)
	0x91, 0x01,    //Output (static constant)

	// Key arrays (6 bytes)
	0x95, 0x06,     // Report Count (6)
	0x75, 0x08,     // Report Size (8)
	0x15, 0x00,     // Log Min (0)
	0x25, 0xF1,     // Log Max (241)
	0x05, 0x07,     // Usage Pg (Key Codes)
	0x19, 0x00,     // Usage Min (0)
	0x29, 0xf1,     // Usage Max (241)
	0x81, 0x00,     // Input: (Data, Array)

	0xC0,            // End Collection

	//consumer report in
	0x05, 0x0C,   // Usage Page (Consumer)
	0x09, 0x01,   // Usage (Consumer Control)
	0xA1, 0x01,   // Collection (Application)
	0x85, HID_REPORT_ID_CONSUME_CONTROL_INPUT,   //     Report Id
	0x75,0x10,     //global, report size 16 bits
	0x95,0x01,     //global, report count 1
	0x15,0x01,     //global, min  0x01
	0x26,0x8c,0x02,  //global, max  0x28c
	0x19,0x01,     //local, min   0x01
	0x2a,0x8c,0x02,  //local, max    0x28c
	0x81,0x00,     //main,  input data varible, absolute
	0xc0,        //main, end collection

};

// HID External Report Reference Descriptor for report map
static u16 extServiceUUID;


/////////////////////////////////////////////////////////
static const  u8 my_OtaServiceUUID[16]				= TELINK_OTA_UUID_SERVICE;
static u8 my_OtaData 						        = 0x00;

static const u8  my_MicName[] = {'M', 'i', 'c'};
static const u8  my_SpeakerName[] = {'S', 'p', 'e', 'a', 'k', 'e', 'r'};
static const u8  my_OtaName[] = {'O', 'T', 'A'};


// Include attribute (Battery service)
static const u16 include[3] = {BATT_PS_H, BATT_LEVEL_INPUT_CCB_H, SERVICE_UUID_BATTERY};


//// GAP attribute values
static const u8 my_devNameCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
	U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
};
static const u8 my_appearanceCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
	U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
};
static const u8 my_periConnParamCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
	U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
};


//// GATT attribute values
static const u8 my_serviceChangeCharVal[5] = {
	CHAR_PROP_INDICATE,
	U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
	U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
};


//// device Information  attribute values
static const u8 my_PnCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(DeviceInformation_pnpID_DP_H), U16_HI(DeviceInformation_pnpID_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PNP_ID), U16_HI(CHARACTERISTIC_UUID_PNP_ID)
};


//// HID attribute values
static const u8 my_hidProtocolModeCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_PROTOCOL_MODE_DP_H), U16_HI(HID_PROTOCOL_MODE_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE), U16_HI(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE)
};
static const u8 my_hidbootKeyInReporCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(HID_BOOT_KB_REPORT_INPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT)
};
static const u8 my_hidbootKeyOutReporCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_BOOT_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_OUTPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT)
};
static const u8 my_hidReportCCinCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(HID_CONSUME_REPORT_INPUT_DP_H), U16_HI(HID_CONSUME_REPORT_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYinCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(HID_NORMAL_KB_REPORT_INPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYoutCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_NORMAL_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_OUTPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportMapCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(HID_REPORT_MAP_DP_H), U16_HI(HID_REPORT_MAP_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_REPORT_MAP), U16_HI(CHARACTERISTIC_UUID_HID_REPORT_MAP)
};
static const u8 my_hidinformationCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(HID_INFORMATION_DP_H), U16_HI(HID_INFORMATION_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_INFORMATION), U16_HI(CHARACTERISTIC_UUID_HID_INFORMATION)
};
static const u8 my_hidCtrlPointCharVal[5] = {
	CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(HID_CONTROL_POINT_DP_H), U16_HI(HID_CONTROL_POINT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_HID_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_HID_CONTROL_POINT)
};


//// Battery attribute values
static const u8 my_batCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(BATT_LEVEL_INPUT_DP_H), U16_HI(BATT_LEVEL_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_BATTERY_LEVEL), U16_HI(CHARACTERISTIC_UUID_BATTERY_LEVEL)
};


//// OTA attribute values
static const u8 my_OtaCharVal[19] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(OTA_CMD_OUT_DP_H), U16_HI(OTA_CMD_OUT_DP_H),
	TELINK_SPP_DATA_OTA,
};

int otaWrite4_test_use(void * p)
{
	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;                //connHandle:0x0082, ledID:0x03, ON:0x01. //demo test data: 82 00 03 01
																		 //connHandle:0x0082, ledID:0x03, ON:0x00. //demo test data: 82 00 03 00
	int led_switch_id = req->dat[2];
	int led_status = req->dat[3];

	u16 connHandle =  req->dat[0] | (req->dat[1]<<8);
	u8 idx = connHandle & (BLMS_MAX_CONN_NUM-1);
	st_llms_conn_t* pc = (st_llms_conn_t*)&blms[idx];

	if(pc->role == LL_ROLE_MASTER){
		u8  dat[32] = {0};
		//so any ATT data before service discovery will be dropped
		dev_char_info_t* dev_info = dev_char_info_search_by_connhandle (connHandle);

		if(dev_info){
			u16 conn_ota_w_hdl = dev_info->char_handle[2];

			//write cmd
			u8 led_ctrl[2];
			led_ctrl[0] = led_switch_id;
			led_ctrl[1] = led_status;

			att_req_write_cmd (dat, conn_ota_w_hdl, (u8 *)led_ctrl, 2);
			blms_ll_pushTxfifo (connHandle, dat);

			////////////////////////// delete smp key //////////////////////////////
			if(!led_status){
				u32 delete_status = blms_smp_param_delete_by_Address(dev_info->mac_adrType, dev_info->mac_addr);
				if(delete_status){
					blms_ll_disconnect(pc->connHandle, HCI_ERR_REMOTE_USER_TERM_CONN);
					//printf("delete spm key\n");
				}
			}
		}

	}


	return 0;
}


// TM : to modify
static const attribute_t my_Attributes[] = {

	{ATT_END_H - 1, 0,0,0,0,0},	// total num of attribute


	// 0001 - 0007  gap
	{7,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gapServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(&my_characterUUID), (u8*)(my_devNameCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(&my_devNameUUID), (u8*)(my_devName), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(&my_characterUUID), (u8*)(my_appearanceCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(&my_appearanceUIID), 	(u8*)(&my_appearance), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(&my_characterUUID), (u8*)(my_periConnParamCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(&my_periConnParamUUID), 	(u8*)(&my_periConnParameters), 0},


	// 0008 - 000b gatt
	{4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gattServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_serviceChangeCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(&serviceChangeUUID), 	(u8*)(&serviceChangeVal), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0},


	// 000c - 000e  device Information Service
	{3,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_devServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(&my_characterUUID), (u8*)(my_PnCharVal), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0},


	/////////////////////////////////// 4. HID Service /////////////////////////////////////////////////////////
	// 000f
	//{27, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_hidServiceUUID), 0},
	{HID_CONTROL_POINT_DP_H - HID_PS_H + 1, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_hidServiceUUID), 0},

	// 0010  include battery service
	{0,ATT_PERMISSIONS_READ,2,sizeof(include),(u8*)(&hidIncludeUUID), 	(u8*)(include), 0},

	// 0011 - 0012  protocol mode
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidProtocolModeCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidProtocolModeCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2, sizeof(protocolMode),(u8*)(&hidProtocolModeUUID), 	(u8*)(&protocolMode), 0},	//value

	// 0013 - 0015  boot keyboard input report (char-val-client)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyInReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyInReporCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(bootKeyInReport),(u8*)(&hidbootKeyInReportUUID), 	(u8*)(&bootKeyInReport), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(bootKeyInReportCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(bootKeyInReportCCC), 0},	//value

	// 0016 - 0017   boot keyboard output report (char-val)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyOutReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyOutReporCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2, sizeof(bootKeyOutReport), (u8*)(&hidbootKeyOutReportUUID), 	(u8*)(&bootKeyOutReport), 0},	//value


	// 0018 - 001b. consume report in: 4 (char-val-client-ref)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportCCinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportCCinCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2, sizeof(reportConsumerControlIn),(u8*)(&hidReportUUID), 	(u8*)(reportConsumerControlIn), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportConsumerControlInCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(reportConsumerControlInCCC), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefConsumerControlIn),(u8*)(&reportRefUUID), 	(u8*)(reportRefConsumerControlIn), 0},	//value

	// 001c - 001f . keyboard report in : 4 (char-val-client-ref)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYinCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2, sizeof(reportKeyIn),(u8*)(&hidReportUUID), 	(u8*)(reportKeyIn), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyInCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(reportKeyInCCC), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyIn),(u8*)(&reportRefUUID), 	(u8*)(reportRefKeyIn), 0},	//value

	// 0020 - 0022 . keyboard report out: 3 (char-val-ref)
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYoutCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYoutCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyOut),(u8*)(&hidReportUUID), 	(u8*)(reportKeyOut), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyOut),(u8*)(&reportRefUUID), 	(u8*)(reportRefKeyOut), 0},	//value


	// 0023 - 0025 . report map: 3
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportMapCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportMapCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(reportMap),(u8*)(&hidReportMapUUID), 	(u8*)(reportMap), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(extServiceUUID),(u8*)(&extReportRefUUID), 	(u8*)(&extServiceUUID), 0},	//value

	// 0026 - 0027 . hid information: 2
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidinformationCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidinformationCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2, sizeof(hidInformation),(u8*)(&hidinformationUUID), 	(u8*)(hidInformation), 0},	//value

	// 0028 - 0029 . control point: 2
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_hidCtrlPointCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidCtrlPointCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_WRITE,2, sizeof(controlPoint),(u8*)(&hidCtrlPointUUID), 	(u8*)(&controlPoint), 0},	//value

	////////////////////////////////////// Battery Service /////////////////////////////////////////////////////
	// 002a - 002d
	{4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_batServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_batCharVal),(u8*)(&my_characterUUID), (u8*)(my_batCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_batVal),(u8*)(&my_batCharUUID), 	(u8*)(my_batVal), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(batteryValueInCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)(batteryValueInCCC), 0},	//value

	////////////////////////////////////// OTA /////////////////////////////////////////////////////
	// 002e - 0031
	{4,ATT_PERMISSIONS_READ, 2,16,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_OtaServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ, 2, sizeof(my_OtaCharVal),(u8*)(&my_characterUUID), (u8*)(my_OtaCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(&my_OtaUUID),	(&my_OtaData), &otaWrite4_test_use, &otaRead},			//value
	{0,ATT_PERMISSIONS_READ, 2,sizeof (my_OtaName),(u8*)(&userdesc_UUID), (u8*)(my_OtaName), 0},

};

void	my_att_init (void)
{
	bls_att_setAttributeTable ((u8 *)my_Attributes);
}
