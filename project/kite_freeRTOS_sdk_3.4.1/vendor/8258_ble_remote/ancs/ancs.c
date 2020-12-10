/*********************************************************************************************

***********
 * @file     ancs.c
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

#include "ancs.h"

///////////////////////////////////////////////////////////////////////////
//					Local Variables
///////////////////////////////////////////////////////////////////////////
/*
 * Packet  Attribute command: FIND_BY_TYPE_VALUE_REQUEST
 * Used to obtain the handles of attributes that have 16 bit uuid type and attribute value.
 */

void ancs_sendCmdReq(u8 *p, u8 req);
void ancs_setBundleIdLen();

////////////START///////////////ANCS build ios service list /////////////START//////////
void setObtainServiceFlag(att_enableAncsFlow flag){
	obtainAttEnable = flag;
}

att_enableAncsFlow getObtainServiceFlag(){
	return obtainAttEnable;
}

u8 att_getAncsSrvHdByUUID(u8 *p_uuid){

	if(memcmp(p_uuid, ancs_control_point_uuid, 16) == 0){
		return 1;
	}else if(memcmp(p_uuid, ancs_notify_source_uuid, 16) == 0){
		return 2;
	}else if(memcmp(p_uuid, ancs_data_source_uuid, 16) == 0){
		return 3;
	}
		
	return 0;
}

void att_finishReadAttSrv(){///excuted once after searching all att handles from master
	
	///initial some fixed parameters of ancsDataSrcParm.getNotifyAtt_writeReq
	ancsDataSrcParm.getNotifyAtt_writeReq.dma_len	= sizeof(ancsDataSrcParm.getNotifyAtt_writeReq) - 4;
	ancsDataSrcParm.getNotifyAtt_writeReq.type		= 0x02;
	ancsDataSrcParm.getNotifyAtt_writeReq.rf_len 	= sizeof(ancsDataSrcParm.getNotifyAtt_writeReq) - 6;
	ancsDataSrcParm.getNotifyAtt_writeReq.l2capLen	= sizeof(ancsDataSrcParm.getNotifyAtt_writeReq) - 10;
	ancsDataSrcParm.getNotifyAtt_writeReq.chanId 	= 0x04;
	ancsDataSrcParm.getNotifyAtt_writeReq.opcode 	= ATT_OP_WRITE_REQ;

}

void att_sendReadByType_req(u16 startHandle, u16 endingHandle, u16 uuid){
	
	rf_packet_att_readByType_t			att_readByType_req;

	att_readByType_req.dma_len 			= sizeof(rf_packet_att_readByType_t);
	att_readByType_req.type				= 0x02;
	att_readByType_req.rf_len			= 0x0B;
	att_readByType_req.l2capLen			= 0x07;
	att_readByType_req.chanId			= 0x04;
	att_readByType_req.opcode			= ATT_OP_READ_BY_TYPE_REQ;
	att_readByType_req.startingHandle 	= startHandle;
	att_readByType_req.startingHandle1 	= startHandle >> 8;
	att_readByType_req.endingHandle 	= endingHandle;
	att_readByType_req.endingHandle1 	= endingHandle >> 8;	
	att_readByType_req.attType[0]		= uuid;
	att_readByType_req.attType[1]		= uuid >> 8;

	//u8 irq_val = irq_disable();
	bls_ll_pushTxFifo(BLS_CONN_HANDLE, (u8*)(&att_readByType_req) + 4);
	//irq_restore(irq_val);
}

void att_readByTypeReq_errHandle(){
	if(ancsFindSrvFlow.curStartHandle < ancsFindSrvFlow.dbHandleAncsSrv.endingHandle){
		att_sendReadByType_req(ancsFindSrvFlow.curStartHandle, ancsFindSrvFlow.dbHandleAncsSrv.endingHandle, GATT_UUID_CHARACTER);
	}else{///ancsFindSrvFlow.curStartHandle equal or larger than ancsFindSrvFlow.dbHandleAncsSrv.endingHandle

	}
}

void att_procErrorRsp(u8 *p){
	rf_packet_att_errRsp_t *err_rsp = (rf_packet_att_errRsp_t *)p;

	if(err_rsp->errReason == (ATT_ERR_ATTR_NOT_FOUND)){///att not found
		if(err_rsp->errOpcode == ATT_OP_READ_BY_TYPE_REQ){
			att_readByTypeReq_errHandle();
		}
	}
	else{
		///others error reason handle
	}
}

u8 att_searchDb128(u8 index){

	if((ancsFindSrvFlow.db128[index].startHandle) && ((ancsFindSrvFlow.db128[index].startHandle) < (ancsFindSrvFlow.db128[index].endingHandle))){
		return 1;
	}else{
		return 0;
	}
}

void att_storeDb128Data(u16 startHd, u16 endingHd, u8 bRet){

	u8 curNum = 0;

	if(att_searchDb128(bRet - 1)){
		return;
	}

	if((ancsFindSrvFlow.i128) < ATT_DB_UUID128_NUM){
		curNum = ancsFindSrvFlow.i128;

		ancsFindSrvFlow.db128[curNum].startHandle	= startHd;
		ancsFindSrvFlow.db128[curNum].endingHandle	= endingHd;

		ancsFindSrvFlow.i128++;
	}

	if(bRet == 1){
		ancsFindSrvFlow.hdControlPoint = curNum;
	}else if(bRet == 2){
		ancsFindSrvFlow.hdNotifySource = curNum;
	}else if(bRet == 3){
		ancsFindSrvFlow.hdDataSource = curNum;
	}
}

void att_procReadByTypeRsp(u8 *p){

	// process response data
	u16 tmpStartHandle = 0, tmpEndingHandle = 0;
	
	rf_packet_att_readByTypeRsp_t *p_rsp = (rf_packet_att_readByTypeRsp_t *) p;
	
	if (p_rsp->datalen == 21)		//uuid128
	{
		tmpEndingHandle	= p_rsp->data[3] + ((p_rsp->data[4]) << 8);
		tmpStartHandle	= p_rsp->data[0] + ((p_rsp->data[1]) << 8);
		u8 ret = att_getAncsSrvHdByUUID(p_rsp->data + 5);

		if(ret){
			att_storeDb128Data(tmpStartHandle, tmpEndingHandle, ret);
		}

		setObtainServiceFlag(OBTAIN_ATT_READ_BY_TYPE);
	}
	
	ancsFindSrvFlow.curStartHandle = tmpEndingHandle + 1;

	///manage the start handle and ending handle for next event
	if((ancsFindSrvFlow.dbHandleAncsSrv.endingHandle < ancsFindSrvFlow.curStartHandle)){
		///judgement current ending handle is the last one
		///not set to 0 for avoid the conn_para_update send more
		setObtainServiceFlag(OBTAIN_ATT_FIND_INFO);
		att_finishReadAttSrv();
	}
}

void att_sendFindInfo_req(){
	rf_pkt_custom_find_info_req myPkt;
	u32 conn_interval = bls_ll_getConnectionInterval() * 1250;

	if((ancsDataSrcParm.tick_reqTimeOut & 0x01) && (!clock_time_exceed(ancsDataSrcParm.tick_reqTimeOut, 3 * conn_interval))){
		return;
	}
	myPkt.dma_len 		= sizeof(rf_pkt_custom_find_info_req) - 4;
	myPkt.type			= 0x02;
	myPkt.rf_len		= sizeof(rf_pkt_custom_find_info_req) - 5;
	myPkt.l2capLen		= myPkt.rf_len - 4;
	myPkt.chanId		= 0x0004;
	myPkt.opcode		= ATT_OP_FIND_INFO_REQ;
	myPkt.startHandle	= ancsFindSrvFlow.db128[ancsFindSrvFlow.hdControlPoint + ancsFindSrvFlow.findInfoFlow].endingHandle + 1;
	myPkt.endingHandle	= ancsFindSrvFlow.db128[ancsFindSrvFlow.hdControlPoint + ancsFindSrvFlow.findInfoFlow].endingHandle + 1;

	//u8 irq_val = irq_disable();
	if(bls_ll_pushTxFifo(BLS_CONN_HANDLE,(u8*)(&myPkt) + 4)){
		
		ancsDataSrcParm.tick_reqTimeOut = clock_time() | 1;
		ancsFindSrvFlow.findInfoFlow = BIT(ancsFindSrvFlow.findInfoFlow);

		if(ancsFindSrvFlow.findInfoFlow > 2){
			ancsDataSrcParm.tick_reqTimeOut = 0x00;
			setObtainServiceFlag(OBTAIN_ATT_WRITE_NOTIFY_SRC_HANDLE);
		}		
	}
	//irq_restore(irq_val);
}

/*
void att_sendReadReq(u16 handle){
	u8 p[13] = {0};

	p[0] 	= sizeof(p) - 4;
	p[1] 	= 0x00;
	p[2] 	= 0x00;
	p[3] 	= 0x00;
	p[4] 	= 0x02;
	p[5]	= 7;
	p[6] 	= 3;
	p[7] 	= 0;
	p[8] 	= 4;
	p[9] 	= 0;
	p[10] 	= ATT_OP_READ_REQ;
	p[11] 	= handle;
	p[12] 	= handle >> 8;

	bls_ll_pushTxFifo(BLS_CONN_HANDLE, p + 4);
}
*/

u8 ancs_sendInitWriteReq(u16 handle){

	init_writeReq[11] = handle;
	init_writeReq[12] = handle >> 8;

	//u8 irq_val = irq_disable();
	u8 ret = bls_ll_pushTxFifo(BLS_CONN_HANDLE, init_writeReq + 4);
	//irq_restore(irq_val);
	return ret;
}

void ancs_buildServiceTable(){

	switch(getObtainServiceFlag()){
	case OBTAIN_ATT_ANCS_SERVICE_DISABLE:{
		setObtainServiceFlag(OBTAIN_ATT_READ_BY_TYPE);
		ancsFindSrvFlow.curStartHandle 	= ancsFindSrvFlow.dbHandleAncsSrv.startHandle;
		ancsFindSrvFlow.curEndingHandle	= ancsFindSrvFlow.dbHandleAncsSrv.endingHandle;
		break;
	}
	case OBTAIN_ATT_READ_BY_TYPE:{

		if(ancsFindSrvFlow.curStartHandle == ancsFindSrvFlow.curEndingHandle){
			setObtainServiceFlag(OBTAIN_ATT_FIND_INFO);
			att_finishReadAttSrv();
		}else{
			att_sendReadByType_req(ancsFindSrvFlow.curStartHandle, ancsFindSrvFlow.curEndingHandle, GATT_UUID_CHARACTER);
		}

		break;
	}
	case OBTAIN_ATT_FIND_INFO:{
		att_sendFindInfo_req();
		break;
	}
	case OBTAIN_ATT_WRITE_NOTIFY_SRC_HANDLE:{
		///as we get the ancs handles,we need to send write req to the NotifySrc Handle
		///if not, we couldn't get the detail information by AttUID

		if(ancs_sendInitWriteReq(ancsFindSrvFlow.db128[ancsFindSrvFlow.hdNotifySource].endingHandle + 1)){
			ancsFindSrvFlow.writeReqFlow |= ANCS_SEND_INIT_WRITE_REQ;
			setObtainServiceFlag(OBTAIN_ATT_ANCS_SERVICE_ENABLED);
		}

		///timer for filter notify after write_req_cmd
		ancsNotifySrcParm.ancs_clrTick = clock_time();

		break;
	}
	case OBTAIN_ATT_ANCS_SERVICE_ENABLED:
		break;
	default:
		break;
	}
}
////////////END///////////////ANCS build ios service list /////////////END//////////
/*
 * Return Current connection state.
 * if return ANCS_CONNECTION_ESTABLISHED, indicate establish connection finished.
 * 返回 当前的连接状态。
 * 如果返回值为ANCS_CONNECTION_ESTABLISHED = 4时，表明连接成功。否则，连接过程正在进行。
 * */
ancs_sevice_state_t ancsGetConnState(){
	return ancsStateFlag.ancs_system_state;
}
/*
 * 返回当前系统的ANCS功能是否有效
 * */
u8 ancsFuncIsEn(){
	return ancsStateFlag.ancs_func_valid;
}

void ancsFuncSetEnable(u8 enable){
	ancsStateFlag.ancs_func_valid = enable;
	ancs_setBundleIdLen();
}

void ancs_addNotifyDataTbl(ancs_notifySrc_type_t* p){

	if(ancsNotifySrcParm.notifyTblEndPtr == MAX_NOTIFY_DATA_LIST_NUM){
		ancsNotifySrcParm.notifyTblEndPtr = 0;
	}
	
	memcpy(&(ancsNotifySrcParm.notifyTbl[ancsNotifySrcParm.notifyTblEndPtr]), p, sizeof(ancs_notifySrc_type_t));

	ancsNotifySrcParm.notifyTblEndPtr++;
}

ancs_notifySrc_type_t ancs_getCurNotify(u8 curPtr){
	
	return ancsNotifySrcParm.notifyTbl[curPtr];
}

void ancs_getCurNotifyAttsData(){
	
	if(ancsFindSrvFlow.writeReqFlow & ANCS_REQ_GET_NOTIFY_ATT){
		u32 conn_interval = bls_ll_getConnectionInterval() * 1250;
		if((ancsDataSrcParm.tick_reqTimeOut & 0x01) && (!clock_time_exceed(ancsDataSrcParm.tick_reqTimeOut, 6 * conn_interval))){
			ancsFindSrvFlow.writeReqFlow &= (~ANCS_REQ_GET_NOTIFY_ATT);
		}else{
			ancs_notifySrc_type_t tmp = ancs_getCurNotify(ancsNotifySrcParm.notifyTblEndPtr - 1);

			ancs_sendCmdReq((u8*)&tmp, ancsFindSrvFlow.writeReqFlow);
			ancsDataSrcParm.ancs_no_latency = 1;

			ancsDataSrcParm.tick_reqTimeOut = clock_time() | 0x01;

		}
	}
}

void ancs_setBundleIdLen(){
	u32 i;
	for(i = 0; i < 100; i++){
		if(memcmp(appBundleId[i], appId_null, 5) == 0){
			break;
		}
	}

	ancsDataSrcParm.num_appBundleId = i;
}

u8 ancs_findAppBundleId(u8 notifyId[], u16 len){
	for(u32 i = 0; i < (ancsDataSrcParm.num_appBundleId); i++){
		if(memcmp(appBundleId[i], notifyId, len) == 0){
			return i + 1;
		}
	}

	return 0;
}

u8 ancs_findCurNotifyIdentifier(){

	u8 ret = 0;

	if((ancsNotifySrcParm.curNotifyAttsIdTbl.appId.attId == ATT_ID_APP_IDENTIFIER) && (ancsNotifySrcParm.curNotifyAttsIdTbl.message.attId == ATT_ID_MESSAGE)){

		ret = ancs_findAppBundleId(ancsNotifySrcParm.curNotifyAttsIdTbl.appId.data, ancsNotifySrcParm.curNotifyAttsIdTbl.appId.len);

		if(ret){

			printf("current notify detail is :");
			array_printf(ancsNotifySrcParm.curNotifyAttsIdTbl.message.data, 48);
			switch(ret){
				case BUNDLE_ID_NULL:{
					break;
				}
				case BUNDLE_ID_CALL:{
#if UART_PRINT_DEBUG_ENABLE
					printf("Enter_ID_CALL");
#endif



					break;
				}
				case BUNDLE_ID_QQ:{

					break;
				}
				case BUNDLE_ID_SMS:{

					break;
				}
				case BUNDLE_ID_WECHAT:{

					printf("Wechat is :");
					array_printf(ancsNotifySrcParm.curNotifyAttsIdTbl.message.data, 48);

					break;
				}
				default:
					break;
			}
		}

		if(ret){
			if((ret != BUNDLE_ID_CALL) && (ret != BUNDLE_ID_SMS)){
				ancsNotifySrcParm.curNotifyAttsIdTbl.title.len = 0;
			}else{
				ancsNotifySrcParm.curNotifyAttsIdTbl.message.len = 0;
			}

			memset(&(ancsNotifySrcParm.curNotifyAttsIdTbl), 0x00, sizeof(ancsNotifySrcParm.curNotifyAttsIdTbl));
			ancsDataSrcParm.ancs_no_latency = 0;
		}
	}

	return ret;
}

bool ancs_findAncsService(){
	rf_pkt_find_by_type_t myPkt;

	myPkt.dma_len 		= sizeof(rf_pkt_find_by_type_t) - 4;
	myPkt.type			= 0x02;
	myPkt.rf_len		= sizeof(rf_pkt_find_by_type_t) - 6;
	myPkt.l2cap_len		= sizeof(rf_pkt_find_by_type_t) - 10;
	myPkt.chan_id		= 0x04;
	myPkt.opcode		= ATT_OP_FIND_BY_TYPE_VALUE_REQ;
	myPkt.start_handle	= 0x0001;
	myPkt.ending_handle	= 0xffff;
	myPkt.service		= 0x2800;

	memcpy(myPkt.uuid, ancs_primary_service_uuid, 16);

	//u8 irq_val = irq_disable();
	u8 ret = bls_ll_pushTxFifo(BLS_CONN_HANDLE, (u8*)(&myPkt) + 4);
	//irq_restore(irq_val);

	return ret;
}
  
/*
 * 建立基于 ANCS 的连接，主要是BLE协议栈层的数据交互。
 * 返回当前连接的状态。
 * */
ancs_sevice_state_t ancsEstshConnection(){

	switch (ancsStateFlag.ancs_system_state){
		case ANCS_CONNECTION_ESTABLISHED:
			break;
		case ANCS_NO_STATE:{
			ancs_findAncsService();
			ancsStateFlag.ancs_system_state = ANCS_GET_START_HANDLE;
			break;
		}
		case ANCS_GET_START_HANDLE_FINISH:
			if(getObtainServiceFlag() < OBTAIN_ATT_ANCS_SERVICE_ENABLED){
				ancs_buildServiceTable();
			}else{
				ancsStateFlag.ancs_system_state = ANCS_CONNECTION_ESTABLISHED;
			}
			break;
		default:
			break;
	}

	return ancsStateFlag.ancs_system_state;
}

/*
请求命令,用于获取当前收到新通知的详细内容
命令发送具体格式请参考相关文档
这里要注意length  为u16  类型,ATT_CMD_MESSAGE_LENGTH这个表示想获取内容
最大长度,若想修改内容长度,建议直接修改ATT_CMD_MESSAGE_LENGTH的值
但是另一方面要考虑这个值一旦变大,会影响到ram size,别只
修改ATT_ID_MESSAGE  的内容长度,如果TITLE长度也没对应修改
会导致获取内容出错
*/
void ancs_sendCmdReq(u8 *p, u8 req){

	if(req & ANCS_REQ_GET_NOTIFY_ATT){
		
		ancs_notifySrc_type_t 	*p_notifyData = (ancs_notifySrc_type_t *) p;
		ancs_getNotify_atts 	getNotifyAtts;

		getNotifyAtts.cmdId		= CMD_ID_GET_NOTIFY_ATTS;
		getNotifyAtts.attIds[0]	= ATT_ID_APP_IDENTIFIER;///not allow to set length for this attID
		getNotifyAtts.attIds[1]	= ATT_ID_TITLE;
		getNotifyAtts.attIds[2]	= ATT_CMD_TITLE_LENGTH;
		getNotifyAtts.attIds[3]	= ATT_CMD_TITLE_LENGTH >> 8;///length settle 16bits
		getNotifyAtts.attIds[4]	= ATT_ID_MESSAGE;
		getNotifyAtts.attIds[5]	= (u8)ATT_CMD_MESSAGE_LENGTH;
		getNotifyAtts.attIds[6]	= ATT_CMD_MESSAGE_LENGTH >> 8;
		
		memcpy(getNotifyAtts.notifyUid, p_notifyData->NotifyUID, 4);

		ancsDataSrcParm.getNotifyAtt_writeReq.handle = ancsFindSrvFlow.db128[ancsFindSrvFlow.hdControlPoint].endingHandle;
		memcpy(&(ancsDataSrcParm.getNotifyAtt_writeReq.attIdInfo), &getNotifyAtts.cmdId, sizeof(ancs_getNotify_atts));

		//u8 irq_val = irq_disable();
		bls_ll_pushTxFifo(BLS_CONN_HANDLE, (u8*)(&(ancsDataSrcParm.getNotifyAtt_writeReq)) + 4);
		//irq_restore(irq_val);

		ancsFindSrvFlow.writeRspFlag |= ANCS_NEWS_GET_NOTIFY_ATT;///mark
		ancsFindSrvFlow.writeReqFlow &= (~ANCS_REQ_GET_NOTIFY_ATT);
	}else if(req & ANCS_REQ_GET_APP_ATT){
		///proc ANCS_REQ_GET_APP_ATT

		ancsFindSrvFlow.writeRspFlag |= ANCS_NEWS_GET_APP_ATT;
	}
}

///judge that the notify packet comes from NP were finished or not
u8	searchAttCmd(){
	
	for(s32 i = 0; i < ancsNotifySrcParm.myBufLen; i++){///find out the start index of ancsDataSrcParm.ancs_dataSrcData[]
		if(ancsDataSrcParm.ancs_dataSrcData[i] == ancsDataSrcParm.getNotifyAtt_writeReq.attIdInfo.cmdId){

			if(memcmp(ancsDataSrcParm.ancs_dataSrcData + i + 1, ancsDataSrcParm.getNotifyAtt_writeReq.attIdInfo.notifyUid, 0x04) == 0){

				u8 	validAttIdNum 	= 0;
				u16 lastAttIdLen 	= 0;
				
				for(u16 pos = i + 5; pos < ancsNotifySrcParm.myBufLen;){///try to search last att id

					if(((validAttIdNum == 0) && (ancsDataSrcParm.ancs_dataSrcData[pos] == ATT_ID_APP_IDENTIFIER))
						||((ancsDataSrcParm.ancs_dataSrcData[pos] > ATT_ID_APP_IDENTIFIER) && (ancsDataSrcParm.ancs_dataSrcData[pos] < ATT_ID_RESERVED))){

						u16 cmdDataLen = (ancsDataSrcParm.ancs_dataSrcData[pos + 2] << 8) + ancsDataSrcParm.ancs_dataSrcData[pos + 1];

						if(validAttIdNum && (cmdDataLen > ATT_CMD_MESSAGE_LENGTH)){
							continue;
						}

						///20170323,the max number of att id is 9.
						validAttIdNum++;

						if(validAttIdNum == ATT_ID_NUM){///judge the data all valid
							ancsNotifySrcParm.cmdPktStartIdx = i;///store the start position of appId
							lastAttIdLen = (ancsDataSrcParm.ancs_dataSrcData[pos + 2] << 8) + ancsDataSrcParm.ancs_dataSrcData[pos + 1];

							for(u16 idx = 0; idx < lastAttIdLen; idx++){
								if(ancsDataSrcParm.ancs_dataSrcData[pos + 3 + idx] == 0){
									ancsNotifySrcParm.cmdPktStartIdx = -1;
									return 0;
								}
							}
							return 1;
						}else{
							pos+=3;///att id(1 byte) att len(2 byte)
						}
					}else{
						pos++;
					}
				}
			}
		}
	}

	return 0;
}

void ancs_clrCmdRspParm(){
	ancsNotifySrcParm.myBufLen		= 0;
	ancsNotifySrcParm.cmdPktStartIdx	= -1;
	
	memset(ancsDataSrcParm.ancs_dataSrcData, 0x00, sizeof(ancsDataSrcParm.ancs_dataSrcData));
	
	ancsFindSrvFlow.writeRspFlag &= (~ANCS_NEWS_GET_NOTIFY_ATT);

}

void ancs_getNotifyAttsData(){
	u8 *ptr = ancsDataSrcParm.ancs_dataSrcData + ancsNotifySrcParm.cmdPktStartIdx + 5;
	u8 tmpLen	= *(ptr + 1) + (*(ptr + 2) << 8) + 3;///three bytes for cmdId(1) and length(2)

	memcpy(&(ancsNotifySrcParm.curNotifyAttsIdTbl.appId), ptr, tmpLen);

	ptr += tmpLen;
	tmpLen = *(ptr + 1) + ((*ptr + 2) << 8) + 3;
	memcpy(&(ancsNotifySrcParm.curNotifyAttsIdTbl.title), ptr, tmpLen);

	ptr += tmpLen;
	tmpLen = *(ptr + 1) + ((*ptr + 2) << 8) + 3;
	memcpy(&(ancsNotifySrcParm.curNotifyAttsIdTbl.message), ptr, tmpLen);

	ancs_clrCmdRspParm();

	return;
}

int ancs_parseCmdRspPkt(){
	
	u8	cmdRspType;
	
	cmdRspType 	= ancsDataSrcParm.ancs_dataSrcData[0];
	
	if(ancsNotifySrcParm.cmdPktStartIdx < 0){
		return -2;
	}
	
	ancsStateFlag.ancs_dataSrcRcvNews = 0;

	if(cmdRspType == CMD_ID_GET_NOTIFY_ATTS){
		ancs_getNotifyAttsData();
	}else if(cmdRspType == CMD_ID_GET_APP_ATTS){

	}else{
	
	}

	return -1;
}
	
ancs_categroyID_t ancsNewsType(){
	
	u32 conn_interval = bls_ll_getConnectionInterval() * 1250;

	ancsStateFlag.ancs_notifySrcRcvNews = 0;

	if((ancsDataSrcParm.ancsStable == 0) && ancsDataSrcParm.tick_lastNotifyRev && (ancsNotifySrcParm.myBufLen == 0)
		&& clock_time_exceed(ancsDataSrcParm.tick_lastNotifyRev, 2 * conn_interval)){

		ancsFindSrvFlow.writeReqFlow |= ANCS_REQ_GET_NOTIFY_ATT;
		ancs_getCurNotifyAttsData();
	}
	
	return ancsNotifySrcParm.ancs_lastNotifySrcData.CategroyID;
}

void latency_turn_off_once(){
	extern st_ll_pm_t bltPm;
	bltPm.sys_latency = 0;
	bltPm.latency_off = 1;

}

int ancs_procNews(){

	int ret = 0;
	u32 conn_interval = bls_ll_getConnectionInterval() * 1250;

	if(ancsStateFlag.ancs_notifySrcRcvNews){
		ret = ancsNewsType();
	}else if(ancsStateFlag.ancs_dataSrcRcvNews){
		ret = ancs_parseCmdRspPkt();
	}

	if((ancsDataSrcParm.ancsStable == 0) && (ancsDataSrcParm.tick_lastNotifyRev & 0x01) && clock_time_exceed(ancsDataSrcParm.tick_lastNotifyRev, 10 * conn_interval ) ){
		ancsDataSrcParm.ancsStable = 1;
	}

	ancs_findCurNotifyIdentifier();

	if(ancsDataSrcParm.ancs_no_latency){
		latency_turn_off_once();
	}

	///latency set enable force
	if((ancsDataSrcParm.ancsStable == 1) && ancsDataSrcParm.ancs_no_latency && (ancsDataSrcParm.tick_lastNotifyRev & 0x01)
		&& clock_time_exceed(ancsDataSrcParm.tick_lastNotifyRev, 1 * 1000 * 1000)){
		ancsDataSrcParm.ancs_no_latency = 0;
		ancsDataSrcParm.tick_lastNotifyRev = 0;
	}

	return ret;
}

/*
 * 主要用于处理ANCS连接的过程。
 * 包括两个过程：
 * 1. 建立连接完成之前，建立连接的过程。
 * 2. 建立连接完成之后， 用于获取是否有新的消息，以及获取新的消息的类型。
 * 返回值：
 * 			非0 -- 新消息类型。
 * 		   	0 -- 无新消息。
 * 		   	-1 -- 连接未建立成功。
 * */

void ancsHaveNews ()///ancs send cmd out
{
	if(blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		return;  // no connection
	}

	u32 conn_interval = bls_ll_getConnectionInterval() * 1250;

	if(clock_time_exceed(tick_ancsLoopControl, conn_interval * 3)){
		tick_ancsLoopControl = clock_time();
	}else{
		return;
	}

	if(ancsFuncIsEn()){
		if(ancsGetConnState() < ANCS_CONNECTION_ESTABLISHED){	// establishing connection
			ancsEstshConnection();
		}else if(ancsGetConnState() == ANCS_CONNECTION_ESTABLISHED){
			ancs_procNews();
		}
	}

	return;
}

void ancs_addedEventCmdId(u8 *p){

	memcpy(&ancsNotifySrcParm.ancs_lastNotifySrcData, p, sizeof(ancsNotifySrcParm.ancs_lastNotifySrcData));
	ancsStateFlag.ancs_notifySrcRcvNews = 1;

	if((ancsNotifySrcParm.ancs_lastNotifySrcData.EventID == EventIDNotificationAdded)){
		ancs_addNotifyDataTbl(&(ancsNotifySrcParm.ancs_lastNotifySrcData));
		ancsDataSrcParm.tick_lastNotifyRev = clock_time() | 0x01;

		if(ancsNotifySrcParm.myBufLen && ancsNotifySrcParm.ancs_clrTick){
			if(clock_time_exceed(ancsNotifySrcParm.ancs_clrTick, 500 * 1000)){
				ancsNotifySrcParm.ancs_clrTick 	= 0;
				ancsNotifySrcParm.myBufLen		= 0;
			}
		}

		if((ancsNotifySrcParm.myBufLen == 0) && ancsDataSrcParm.ancsStable){
			ancsFindSrvFlow.writeReqFlow |= ANCS_REQ_GET_NOTIFY_ATT;
			ancs_getCurNotifyAttsData();

		}else{
			ancsNotifySrcParm.ancs_clrTick = clock_time();
		}
	}
}

void ancs_getCmdIdDetailData(u8 *p){
	if(ancsFindSrvFlow.writeRspFlag & (ANCS_NEWS_GET_APP_ATT | ANCS_NEWS_GET_NOTIFY_ATT)){

		rf_pkt_notifyRsp_t *p_rsp 	= (rf_pkt_notifyRsp_t *)p;
		u16 dataLen			= p_rsp->l2capLen - 3;


		if((ancsNotifySrcParm.myBufLen + dataLen) > MAX_DATA_SRC_NUM){
			ancs_clrCmdRspParm();
			return;
		}

		memcpy(ancsDataSrcParm.ancs_dataSrcData + ancsNotifySrcParm.myBufLen, p_rsp->data,dataLen);
		ancsNotifySrcParm.myBufLen += dataLen;

		ancsStateFlag.ancs_dataSrcRcvNews = searchAttCmd();
	}
}

/*
 * 用于处理协议栈回调函数。
 * 接收最新的状态信息。
 * */
void ancsStackCallback(u8 *p){///ancs recieve rsp data from ios and handle them

	rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)p;
	
	switch(req->opcode){
	case ATT_OP_FIND_BY_TYPE_VALUE_RSP:
		if(ancsStateFlag.ancs_system_state == ANCS_GET_START_HANDLE){
			ancsStateFlag.ancs_system_state =  ANCS_GET_START_HANDLE_FINISH;
			ancsFindSrvFlow.dbHandleAncsSrv.startHandle 	= ((req->data[1]) << 8) + (req->data[0]);
			ancsFindSrvFlow.dbHandleAncsSrv.endingHandle	= ((req->data[3]) << 8) + (req->data[2]);
		}
		break;
	case ATT_OP_ERROR_RSP:
		att_procErrorRsp(p);
		if(ancsFindSrvFlow.writeReqFlow & ANCS_SEND_INIT_WRITE_REQ){
			if(ancs_sendInitWriteReq(ancsFindSrvFlow.db128[ancsFindSrvFlow.hdDataSource].endingHandle + 1)){
				ancsFindSrvFlow.writeReqFlow &= (~ANCS_SEND_INIT_WRITE_REQ);
				ancsFindSrvFlow.writeReqFlow |= ANCS_SEND_CMD_REQ;
			}
		}
		break;		
	case ATT_OP_READ_BY_TYPE_RSP:
		att_procReadByTypeRsp(p);
		printf("RBT_Respone %d and %d RBT_Respone\r\n", ancsFindSrvFlow.curStartHandle, ancsFindSrvFlow.curEndingHandle);
		break;
	case ATT_OP_FIND_INFO_RSP:///it maybe need a func to handle the rsp pkt
		break;
	case ATT_OP_WRITE_RSP:
		if(ancsFindSrvFlow.writeReqFlow & ANCS_SEND_INIT_WRITE_REQ){
			if(ancs_sendInitWriteReq(ancsFindSrvFlow.db128[ancsFindSrvFlow.hdDataSource].endingHandle + 1)){
				ancsFindSrvFlow.writeReqFlow &= (~ANCS_SEND_INIT_WRITE_REQ);
				ancsFindSrvFlow.writeReqFlow |= ANCS_SEND_CMD_REQ;
			}
		}
		break;		
	case ATT_OP_HANDLE_VALUE_NOTI:{
		u16 tmpHandle = 0;
		tmpHandle = (req->data[0] | (req->data[1] << 8));
		if(ancsFindSrvFlow.db128[ancsFindSrvFlow.hdNotifySource].endingHandle == tmpHandle){
			ancs_addedEventCmdId(req->data + 2);
			printf("ANCS have cmdID coming...\r\n");
		}
		if(ancsFindSrvFlow.db128[ancsFindSrvFlow.hdDataSource].endingHandle == tmpHandle){
			ancs_getCmdIdDetailData(p);
			printf("ANCS have notify data detail coming...\r\n");
		}
		break;
		}
	}
}

/*
 * ANCS 初始化，用于初始化一些ANCS相关的变量。
 * 一般情况下，在程序上电或者断开连接的情况下会调用。
 * */
void ancsInit(){

	setObtainServiceFlag(OBTAIN_ATT_ANCS_SERVICE_DISABLE);

	memset(&ancsStateFlag, 0x00, sizeof(ancsStateFlag));

	memset(&ancsFindSrvFlow, 0x00, sizeof(ancsFindSrvFlow));
	ancsFindSrvFlow.curStartHandle 	= 0x01;
	ancsFindSrvFlow.curEndingHandle = 0x02;

	memset(&ancsNotifySrcParm, 0x00, sizeof(ancsNotifySrcParm));

	ancsNotifySrcParm.cmdPktStartIdx = -1;

	memset(&ancsDataSrcParm, 0x00, sizeof(ancsDataSrcParm));

	tick_ancsLoopControl = clock_time();
}

////call the function to reject an incoming call or not
///after get the detail title from ancs_findCurNotifyIdentifier()
void ancs_handleIncomingCall(u8 accept){
	if(ancsNotifySrcParm.ancs_lastNotifySrcData.CategroyID == CategroyIDIncomingCall){

		ancs_performNotifyAction 	cmd;
		rf_pkt_perform_action		cmdPkt;

		cmd.cmdId 			= CMD_ID_PERFORM_NOTIFY_ACTION;
		cmd.actionIdValue 	= accept ? ACTION_ID_POSTIVE : ACTION_ID_NEGATIVE;
		memcpy(cmd.notifyUid, ancsNotifySrcParm.ancs_lastNotifySrcData.NotifyUID, sizeof(cmd.notifyUid));

		cmdPkt.dma_len	= sizeof(rf_pkt_perform_action) - 4;
		cmdPkt.type		= 0x02;
		cmdPkt.rf_len 	= sizeof(rf_pkt_perform_action) - 6;
		cmdPkt.l2capLen	= sizeof(rf_pkt_perform_action) - 10;
		cmdPkt.chanId 	= 0x04;
		cmdPkt.opcode 	= ATT_OP_WRITE_REQ;

		cmdPkt.handle = ancsFindSrvFlow.db128[ancsFindSrvFlow.hdControlPoint].endingHandle;
		memcpy(&cmdPkt.cmdInfo, &cmd, sizeof(ancs_performNotifyAction));

		//u8 irq_val = irq_disable();
		bls_ll_pushTxFifo(BLS_CONN_HANDLE, (u8*)(&cmdPkt) + 4);
		//irq_restore(irq_val);

		memset(&(ancsNotifySrcParm.ancs_lastNotifySrcData), 0x00, sizeof(ancsNotifySrcParm.ancs_lastNotifySrcData));
	}else{
		return;
	}
}

