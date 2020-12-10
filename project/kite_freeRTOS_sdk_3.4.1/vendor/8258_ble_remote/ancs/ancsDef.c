/*********************************************************************************************

***********
 * @file     ancsDef.c
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
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////Finding the others bundle id, link to : https://offcornerdev.com/bundleid.html////////////
//////////Finding the others bundle id, link to : https://offcornerdev.com/bundleid.html////////////
//////////Finding the others bundle id, link to : https://offcornerdev.com/bundleid.html////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ancsDef.h"

/**
 * The Write Request is used to request the server to write the value of an attribute and acknowledge that this has been achieved in a Write Response
 */
_attribute_data_retention_ u8 init_writeReq[15] = {
		sizeof(init_writeReq) - 4, 0, 0 , 0,   //dma_length   4 bytes   value = sizeof(pkt_att_read_by_type_req) - 4
		0x02,   		// type :  ll data pdu   1 byte
		sizeof(init_writeReq) - 6, 				// rf_length	1 byte  value = sizeof(pkt_att_read_by_type_req) - 6

		sizeof(init_writeReq) - 10, 0, 			// l2cap_length		2 bytes   value = sizeof(pkt_att_read_by_type_req) - 10
		0x04, 0,			// chanID			2 byte
		ATT_OP_WRITE_REQ,  //opcode  1 byte
		0x24, 0x00, 			// handle
		0x01, 0x00,  			// value
};

_attribute_data_retention_ ancs_state_flag ancsStateFlag;
_attribute_data_retention_ ancs_find_srv_flow ancsFindSrvFlow;
_attribute_data_retention_ ancs_data_src_parm ancsDataSrcParm;
_attribute_data_retention_ att_enableAncsFlow obtainAttEnable;
_attribute_data_retention_ ancs_notify_src_parm ancsNotifySrcParm;

_attribute_data_retention_ u32 tick_ancsLoopControl;
///////////////////////////////////Service UUID////////////////////////////////////////
const u8 ancs_primary_service_uuid[16] =
	{0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79};

const u8 ancs_control_point_uuid[16] =
	{0xD9, 0xD9, 0xAA, 0xFD, 0xBD, 0x9B, 0x21, 0x98, 0xA8, 0x49, 0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69};

const u8 ancs_notify_source_uuid[16] =
	{0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, 0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F};

const u8 ancs_data_source_uuid[16] =
	{0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0xBE, 0xB5, 0x4B, 0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22};

///////////////////////////////////////App Bundle Id///////////////////////////////////
const u8 appId_call[] 			=
	{'c','o','m','.','a','p','p','l','e','.','m','o','b','i','l','e','p','h','o','n','e'};
const u8 appId_weChat[] 		=
	{'c','o','m','.','t','e','n','c','e','n','t','.','x','i','n'};
const u8 appId_SMS[]			=
	{'c','o','m','.','a','p','p','l','e','.','M','o','b','i','l','e','S','M','S'};
const u8 appId_qq[]				=
	{'c','o','m','.','t','e','n','c','e','n','t','.','m','q','q'};
const u8 appId_twitter[]		=
	{'c','o','m','.','a','t','e','b','i','t','s','.','T','w','e','e','t','i','e','2'};
const u8 appId_whatsapp[]		=
	{'n','e','t','.','w','h','a','t','s','a','p','p','.','W','h','a','t','s','A','p','p'};
const u8 appId_instagram[]		=
	{'c','o','m','.','b','u','r','b','n','.','i','n','s','t','a','g','r','a','m'};
const u8 appId_skype[]			=
	{'c','o','m','.','s','k','y','p','e','.','s','k','y','p','e'};
const u8 appId_facebook[]		=
	{'c','o','m','.','f','a','c','e','b','o','o','k','.','F','a','c','e','b','o','o','k'};
const u8 appId_line[]			=
	{'j','p','.','n','a','v','e','r','.','l','i','n','e'};
const u8 appId_null[]			=///this array was the last one for ensure where is the end of appBundleId[]
	{'N','U','L','L'};

const u8 * appBundleId[] = {appId_call,appId_weChat,appId_SMS,appId_qq,appId_twitter,appId_whatsapp,appId_instagram,appId_skype,appId_facebook,\
							appId_line,appId_null};

