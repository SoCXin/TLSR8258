/*********************************************************************************************

***********
 * @file     ancsDef.h
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary

property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor

(Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms

described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information

in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND

is provided.
 *


**********************************************************************************************

*********/

#ifndef ANCSCONSTANTDEF_H_
#define ANCSCONSTANTDEF_H_


#include "tl_common.h"
#include "stack/ble/ble.h"
#define MAX_NOTIFY_DATA_LIST_NUM	4

#define ATT_CMD_TITLE_LENGTH		0x20
#define ATT_CMD_MESSAGE_LENGTH		0x20
//#define DATA_MAX_LEN				0x20
#define ATT_ID_NUM					0x03
#define MAX_DATA_SRC_NUM			(((ATT_CMD_TITLE_LENGTH + 3) * 2) + ATT_CMD_MESSAGE_LENGTH + 30)

#define GET_START_HANDLE_TIMEOUT	1000 * 1000///1000ms
#define BUILD_SERVICE_TIMEOUT		1000 * 1000///1000ms

typedef enum{
	ANCS_NO_STATE = 0,					//init state , not allow to send packet about ancs, expect adv packet.
	ANCS_GET_START_HANDLE,			//have send packet about get ancs starting handle
	ANCS_GET_START_HANDLE_FINISH,		//finish get start handle
	ANCS_WRITE_NOTISRC_CCC,				// write CCC finish , and wait provider send information.
	ANCS_CONNECTION_ESTABLISHED,
}ancs_sevice_state_t;

typedef enum{
	CategroyConnErr = -1,
	CategroyIDOther = 0,
	CategroyIDIncomingCall = 1,
	CategroyIDMissCall = 2,
	CategroyIDVoiceMail = 3,
	CategroyIDSocial	= 4,
	CategroyIDSchedule,
	CategroyIDEmail,
	CategroyIDNews,
	CategroyIDHealthyAndFitness,
	CategroyIDBunissAndFinance,
	CategroyIDLocation,
	CategroyIDEntertainment,
}ancs_categroyID_t;

typedef enum{
	EventIDNotificationAdded,
	EventIDNotificationModified,
	EventIDNotificationRemoved,
}ancs_eventID_t;

typedef enum{
	EventFlagSilent = BIT(0),
	EventFlagImportant = BIT(1),
}ancs_event_flags_t;

typedef enum{
	ATT_ID_APP_IDENTIFIER = 0,
	ATT_ID_TITLE,
	ATT_ID_SUB_TITLE,
	ATT_ID_MESSAGE,
	ATT_ID_MESSAGE_SIZE,
	ATT_ID_DATE,
	ATT_ID_POSITIVE_ACTION_LABEL,
	ATT_ID_NEGATTIVE_ACTION_LABEL,
	ATT_ID_RESERVED,
}ancs_notifyAttIdValues;

typedef enum{
	CMD_ID_GET_NOTIFY_ATTS = 0,
	CMD_ID_GET_APP_ATTS,
	CMD_ID_PERFORM_NOTIFY_ACTION,
}ancs_cmdIdValues;

typedef enum{
	CMD_RSP_PKT_START = BIT(0),
	CMD_RSP_PKT_END,
}ancs_cmdSequence_flag;

typedef enum{
	ACTION_ID_POSTIVE 	= 0,
	ACTION_ID_NEGATIVE	= 1,
}ancs_actionIdValue;

typedef enum{
	BUNDLE_ID_NULL = 0,
	BUNDLE_ID_CALL = 1,
	BUNDLE_ID_WECHAT,
	BUNDLE_ID_SMS,
	BUNDLE_ID_QQ,
	BUNDLE_ID_TWITTER,
	//BUNDLE_ID_FACEBOOK_MSGR,
	BUNDLE_ID_WHATSAPP,
	BUNDLE_ID_INSTAGRAM,
	//BUNDLE_ID_SNAPCHAT,
	//BUNDLE_ID_GOOGLE_TERMINAL,
	//BUNDLE_ID_YOUTUBE,
	BUNDLE_ID_SKYPE,
	BUNDLE_ID_FACEBOOK,
	//BUNDLE_ID_LINKEDIN,
	BUNDLE_ID_LINE,
	//BUNDLE_ID_VENMO,
	//BUNDLE_ID_PINTEREST,
}BUNDLE_ID_INDEX;

typedef struct{
	ancs_eventID_t 			EventID;
	ancs_event_flags_t 		EventFlags;
	ancs_categroyID_t 		CategroyID;
	u8 						CategroyCnt;
	u8 						NotifyUID[4];
}ancs_notifySrc_type_t;

typedef struct{
	u8 cmdId;
	u8 notifyUid[4];
	u8 attIds[7];///this array size can be varied
}ancs_getNotify_atts;

typedef struct{
	u8 cmdId;

}ancs_getApp_atts;

typedef struct{
	u8 	attId;
	u16 len;
	u8 	data[ATT_CMD_TITLE_LENGTH];
}ancs_attId_AppId_Title;

typedef struct{
	u8 	attId;
	u16 len;
	u8 	data[ATT_CMD_MESSAGE_LENGTH];
}ancs_attId_Message;

typedef struct{
	ancs_attId_AppId_Title 	appId;
	ancs_attId_AppId_Title 	title;
	ancs_attId_Message		message;
}ancs_notifyAtts;

typedef struct{
	u8 cmdId;
	u8 notifyUid[4];
	u8 actionIdValue;
}ancs_performNotifyAction;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u8 	handle[2];
	u8 	data[1];
}rf_pkt_notifyRsp_t;

/////////////////////////ATT HANDLE RELEVANT STRUCTURE DEFINATION///////////////////////////////
#define	ATT_DB_UUID128_NUM				4
#define	ATT_DB_HDLIST_MAX_NUM			ATT_DB_UUID128_NUM

#define ANCS_NEWS_GET_NOTIFY_ATT		0x01
#define ANCS_NEWS_GET_APP_ATT			0x02

#define ANCS_SEND_INIT_WRITE_REQ		0x80
#define ANCS_SEND_CMD_REQ				0x40

#define ANCS_REQ_GET_NOTIFY_ATT			0x20
#define ANCS_REQ_GET_APP_ATT			0x10

typedef enum{
	OBTAIN_ATT_ANCS_SERVICE_DISABLE = 0,
	OBTAIN_ATT_READ_BY_TYPE,
	OBTAIN_ATT_FIND_INFO,
	OBTAIN_ATT_WRITE_NOTIFY_SRC_HANDLE,
	OBTAIN_ATT_ANCS_SERVICE_ENABLED,
}att_enableAncsFlow;

typedef struct {
	u16	startHandle;
	u16	endingHandle;
} ancs_att_db_uuid128_t;			//20-byte

typedef struct {
	u16 startHandle;
	u16 endingHandle;
} att_db_handle_element;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust
							//with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u16 handle;
	ancs_getNotify_atts attIdInfo;///12
}rf_pkt_write_req;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust
							//with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u16 handle;
	ancs_performNotifyAction cmdInfo;
}rf_pkt_perform_action;

typedef struct{
	u32 dma_len;            //won't be a fixed number as previous, should adjust
							//with the mouse package number
	u8	type;				//RFU(3)_MD(1)_SN(1)_NESN(1)-LLID(2)
	u8  rf_len;				//LEN(5)_RFU(3)
	u16	l2capLen;
	u16	chanId;
	u8  opcode;
	u16 startHandle;
	u16 endingHandle;
}rf_pkt_custom_find_info_req;

typedef struct{
	u32 dma_len;
	u8	type;
	u8	rf_len;
	u16 l2cap_len;
	u16 chan_id;
	u8	opcode;
	u16 start_handle;
	u16 ending_handle;
	u16 service;
	u8	uuid[16];
}rf_pkt_find_by_type_t;

/////////////////////////ATT HANDLE RELEVANT STRUCTURE DEFINATION///////////////////////////////
typedef struct{
	u8 	ancs_func_valid;
	u8	ancs_dataSrcRcvNews;
	u8	ancs_notifySrcRcvNews;
	ancs_sevice_state_t  ancs_system_state;
}ancs_state_flag;

typedef struct{
	u8 	i128;
	u8	hdDataSource;
	u8	writeReqFlow;
	u8	writeRspFlag;
	u8 	findInfoFlow;
	u8	hdNotifySource;
	u8	hdControlPoint;
	u16 curStartHandle;
	u16 curEndingHandle;

	ancs_att_db_uuid128_t	db128[ATT_DB_UUID128_NUM];
	att_db_handle_element	dbHandleAncsSrv;
}ancs_find_srv_flow;

typedef struct{
	u8 	notifyTblEndPtr;
	s32	cmdPktStartIdx;
	u16 myBufLen;
	u32 ancs_clrTick;
	ancs_notifyAtts curNotifyAttsIdTbl;
	ancs_notifySrc_type_t ancs_lastNotifySrcData;
	ancs_notifySrc_type_t notifyTbl[MAX_NOTIFY_DATA_LIST_NUM];
}ancs_notify_src_parm;

typedef struct{
	u8 ancsStable;
	u8 ancs_no_latency;
	u8 ancs_dataSrcData[MAX_DATA_SRC_NUM];
	u32 tick_lastNotifyRev;
	u32 tick_reqTimeOut;
	u32 num_appBundleId;///4
	rf_pkt_write_req getNotifyAtt_writeReq;///25
}ancs_data_src_parm;

extern u8 init_writeReq[];
extern u32 tick_ancsLoopControl;

extern const u8 ancs_primary_service_uuid[];
extern const u8 ancs_control_point_uuid[];
extern const u8 ancs_notify_source_uuid[];
extern const u8 ancs_data_source_uuid[];

extern const u8 appId_null[];
extern const u8 * appBundleId[];

extern ancs_state_flag 		ancsStateFlag;
extern ancs_find_srv_flow 	ancsFindSrvFlow;
extern ancs_data_src_parm 	ancsDataSrcParm;
extern att_enableAncsFlow 	obtainAttEnable;
extern ancs_notify_src_parm ancsNotifySrcParm;

#endif /* ANCSCONSTANTDEF_H_ */
