/**********************************************************************************************************
 *
 * @file system_ble_task.c
 * 
 * @brief       蓝牙协议栈接口源文件
 * 
 * @author      Chenhao
 * @date        Created:    2019-07-08
 * 

**********************************************************************************************************/
#include "system_ble_task.h"
//#include "system_resource.h"
//#include "../Modules/Ble/ew_mod_ble.h"
#include "stack/ble/ble.h"

/**********************************************************************************************************
  Constants and macros
**********************************************************************************************************/

#define __BLE_DEBUG              ( 0 )
#if __BLE_DEBUG
#define ble_debug(...)           printf(__VA_ARGS__) 
#else
#define ble_debug(...)
#endif

/**********************************************************************************************************
  extern  function
**********************************************************************************************************/

/**********************************************************************************************************
  Private functions
**********************************************************************************************************/


/**********************************************************************************************************
  蓝牙接口: 事件操作
**********************************************************************************************************/

/**
 * @brief 断开连接 
 */
void Ops_BleClose (void)   
{
	bls_ll_terminateConnection (HCI_ERR_REMOTE_USER_TERM_CONN);
}

/**
 * @brief 停止广播 
 */
void Ops_BleAdvStop (void)   
{
	bls_ll_setAdvEnable(0);
}

/**
 * @brief 启动广播 
 */
void Ops_BleAdvStart (void)  
{
	bls_ll_setAdvEnable(1);
}

/**
 * @brief 请求更改连接参数 
 * @param  connIntervalMin: Minimum connection interval.
 * @param  connIntervalMax: Maximum connection interval.
 * @param  connLatency: Connection latency.
 * @param  supTimeout: Supervision timeout.
 */
void Ops_BleUpdateConnParam (u16 connIntervalMin, u16 connIntervalMax, u16 connLatency, u16 supTimeout)
{
	bls_l2cap_requestConnParamUpdate (connIntervalMin, connIntervalMax, connLatency, supTimeout);
}

/**
 * @brief 请求更改MTU 
 * @param  mtuReqSize:请求大小
 */
void Ops_BleUpdateMtu (u16 mtuReqSize)
{
	blc_att_setRxMtuSize(mtuReqSize);
}

/**
 * @brief 写特征值8002 
 */
void Ops_Ble8002CharacterSendData (u8 *pData, u16 Len)
{
	ble_sts_t bret = BLE_SUCCESS;
	u8 *	  pt = pData;
	while(Len)
	{
		if( Len > 20 ){
			bret = bls_att_pushIndicateData(27, pt, 20);
			Len -= 20;
			pt+=20;
		}
		else{
			bret = bls_att_pushIndicateData(27, pt, Len);
			Len = 0;
			pt = pt+Len;
			Cb_BleOn8002CharacterSendDataComplete();
		}
	}
	return;
}

/**
 * @brief 写特征值8004 
 */
void Ops_Ble8004CharacterSendData (u8 *pData, u16 Len)
{
	ble_sts_t bret = BLE_SUCCESS;
	u8 *	  pt = pData;
	while(Len)
	{
		if( Len > 20 ){
			bret = bls_att_pushNotifyData(32, pt, 20);
			Len -= 20;
			pt+=20;
		}
		else{
			bret = bls_att_pushNotifyData(32, pt, Len);
			Len = 0;
			pt = pt+Len;
			Cb_BleOn8004CharacterSendDataComplete();
		}
	}
	return;
}

/**
 * @brief 关闭BLE
 */
void Ops_BleShutdown (void)  
{
	cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_PAD, 0);
}

/**
 * @brief 开启BLE 
 */
void Ops_BleShutup (void)  
{

}

/**
 * @brief ANCS:获取下一条ancs
 */
void Ops_BleGetNextAncs (void)  
{

}

/**
 * @brief ANCS:控制来电接通 :NOTIF_ACTION_ID_POSITIVE
 * @param  notificationUID:唯一标示该消息
 */
void Ops_BleAcceptIncomingCall (u32 notificationUID)
{
  

}

/**
 * @brief ANCS:控制来电挂断: NOTIF_ACTION_ID_NEGATIVE
 * @param  notificationUID:唯一标示该消息
 */
void Ops_BleRejectIncomingCall (u32 notificationUID)
{

}

/**
 * @brief AMS: EU订阅
 */
void Ops_BleAmsEuSubscribe (void)  
{

}

/**
 * @brief AMS: 音乐控制 
 * @param  controlCmd: 0:Play 1:Pause 2:TogglePlayPause 3:NextTrack 4:PreviousTrack 5:VolumeUp 6:VolumeDown
 */
void Ops_BleAmsMusicControl (u8 controlCmd)
{

}

/**********************************************************************************************************
  蓝牙接口: 事件回调
**********************************************************************************************************/
/**
 * @brief 蓝牙连接事件 
 */
void Cb_BleOnConnect(void)                          
{

}

/**
 * @brief 蓝牙连接事件  
 * @param  reason: 断连原因
 */
void Cb_BleOnDisConnect(u8 reason)
{

}

/**
 * @brief 蓝牙发现手机ancs服务失败，此次连接当作Android连接
 */
void Cb_BleOnAncsSvcDiscoverFailed(void)        
{

}

/**
 * @brief 蓝牙得到连接参数更改结果 
 * @param  result: 0 失败 1成功
 * @param  connInterval: 最终连接间隔 (ms)
 * @param  connLatency: 最终Latency
 */
void Cb_BleOnUpdateConnParamsResult(u8 result, u16 connInterval, u16 connLatency)
{

}

/**
 * @brief 获取本次连接的mtu值大小
 * @param  mtuSize: 大小
 */
void Cb_BleOnGetMtuSize (u16 mtuSize)
{

}

/**
 * @brief 获取到特征值8001的数据
 */
void Cb_BleOn8001CharacterGetData (u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到特征值8003的数据
 */
void Cb_BleOn8003CharacterGetData (u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到特征值8002写数据完成
 */
void Cb_BleOn8002CharacterSendDataComplete (void)   
{

}

/**
 * @brief 获取到特征值8004写数据完成
 */
void Cb_BleOn8004CharacterSendDataComplete (void)  
{

}

/**
 * @brief 获取到特征值8002的Ntf受到手机端的操作
 * @param  isEnable: 0 禁能 1 使能
 */
void Cb_BleOn8002CharacterNtf (bool isEnable)
{

}

/**
 * @brief 获取到特征值8004的Ntf受到手机端的操作
 * @param  isEnable: 0 禁能 1 使能
 */
void Cb_BleOn8004CharacterNtf (bool isEnable)  
{

}


/**
 * @brief 获取到Ancs头信息
 * @param  notificationUID: 唯一标示该消息
 * @param  eventID: 表示是Added还是Modified还是Removed
 * @param  eventFlags: 如ancs协议
 * @param  categoryID: 消息类型
 * @param  categoryCount: 给定类型中活跃的通知的数量
 * @param  appIdentifier: App ID，这个字符串必须使用NULL结尾
 */
void Cb_BleOnAncsGetHeadInfo (u32 notificationUID,u8 eventID,u8 eventFlags,u8 categoryID,u8 categoryCount, u8* appIdentifier)
{

}

/**
 * @brief 获取到Ancs标题 Title 
 */
void Cb_BleOnAncsGetTitle(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ancs内容Message 
 */
void Cb_BleOnAncsGetMessage(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ancs时间 Date
 */
void Cb_BleOnAncsGetDate(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_PLAYER中 的 AMS_PLAYATTRIBUTE_ID_PLAYBACK_INFO 
        (PlaybackStatePaused=0 PlaybackStatePlaying=1 PlaybackStateRewinding=2 PlaybackStateFastForwarding=3)
 */
void Cb_BleOnAmsGetPlayState(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_PLAYER中 的  的AMS_PLAYATTRIBUTE_ID_VOLUME      
 */
void Cb_BleOnAmsGetVolume(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_TRACK中 的  的AMS_TRACK_ATTRIBUTE_ID_ARTIST     
 */
void Cb_BleOnAmsGetArtist(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_TRACK中 的  的AMS_TRACK_ATTRIBUTE_ID_TITLE   
 */
void Cb_BleOnAmsGetTitle(u8 *pData, u16 Len)
{

}

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_TRACK中 的  的AMS_TRACK_ATTRIBUTE_ID_DURATION   
 */
void Cb_BleOnAmsGetDuration(u8 *pData, u16 Len)
{

}

/**********************************************************************************************************
  Local variables
**********************************************************************************************************/

/**********************************************************************************************************
  Functions
**********************************************************************************************************/

/**
* @brief (resource) 执行蓝牙操作事件
* @return  false:没有用户调度OPS接口;            true:有用户调度OPS接口.
*/
bool System_Ble_Task_OpsDispatcher (void)
{

    return false;
}

/**
* @brief (resource) 唤醒Ble Task执行 
*/
void System_Ble_Task_WakeUp (void)
{

}

/**********************************************************************************************************
  END FILE
**********************************************************************************************************/
