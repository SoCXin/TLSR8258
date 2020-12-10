/**********************************************************************************************************
 *
 * @file system_ble_task.h
 * 
 * @brief       蓝牙协议栈接口源文件
 * 
 * @author      Chenhao
 * @date        Created:    2019-07-08
 * 
**********************************************************************************************************/
#ifndef __SYSTEM_BLE_TASK_H
#define __SYSTEM_BLE_TASK_H

/**********************************************************************************************************
  Includes
**********************************************************************************************************/
#include "tl_common.h"

/**********************************************************************************************************
  Defines
**********************************************************************************************************/

/**********************************************************************************************************
  Typedefs
**********************************************************************************************************/

/**********************************************************************************************************
  Extern
**********************************************************************************************************/


/**********************************************************************************************************
  Functions 
**********************************************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 蓝牙连接事件 
 */
void Cb_BleOnConnect(void);                          

/**
 * @brief 蓝牙连接事件  
 * @param  reason: 断连原因
 */
void Cb_BleOnDisConnect(u8 reason);

/**
 * @brief 蓝牙发现手机ancs服务失败，此次连接当作Android连接
 */
void Cb_BleOnAncsSvcDiscoverFailed(void);        

/**
 * @brief 蓝牙得到连接参数更改结果 
 * @param  result: 0 失败 1成功
 * @param  connInterval: 最终连接间隔 (ms)
 * @param  connLatency: 最终Latency
 */
void Cb_BleOnUpdateConnParamsResult(u8 result, u16 connInterval, u16 connLatency);

/**
 * @brief 获取本次连接的mtu值大小
 * @param  mtuSize: 大小
 */
void Cb_BleOnGetMtuSize (u16 mtuSize);

/**
 * @brief 获取到特征值8001的数据
 */
void Cb_BleOn8001CharacterGetData (u8 *pData, u16 Len);

/**
 * @brief 获取到特征值8003的数据
 */
void Cb_BleOn8003CharacterGetData (u8 *pData, u16 Len);

/**
 * @brief 获取到特征值8002写数据完成
 */
void Cb_BleOn8002CharacterSendDataComplete (void);   

/**
 * @brief 获取到特征值8004写数据完成
 */
void Cb_BleOn8004CharacterSendDataComplete (void);  

/**
 * @brief 获取到特征值8002的Ntf受到手机端的操作
 * @param  isEnable: 0 禁能 1 使能
 */
void Cb_BleOn8002CharacterNtf (bool isEnable);

/**
 * @brief 获取到特征值8004的Ntf受到手机端的操作
 * @param  isEnable: 0 禁能 1 使能
 */
void Cb_BleOn8004CharacterNtf (bool isEnable);  

/**
 * @brief 获取到Ancs头信息
 * @param  notificationUID: 唯一标示该消息
 * @param  eventID: 表示是Added还是Modified还是Removed
 * @param  eventFlags: 如ancs协议
 * @param  categoryID: 消息类型
 * @param  categoryCount: 给定类型中活跃的通知的数量
 * @param  appIdentifier: App ID，这个字符串必须使用NULL结尾
 */
void Cb_BleOnAncsGetHeadInfo (u32 notificationUID,u8 eventID,u8 eventFlags,u8 categoryID,u8 categoryCount, u8* appIdentifier);

/**
 * @brief 获取到Ancs标题 Title 
 */
void Cb_BleOnAncsGetTitle(u8 *pData, u16 Len);

/**
 * @brief 获取到Ancs内容Message 
 */
void Cb_BleOnAncsGetMessage(u8 *pData, u16 Len);

/**
 * @brief 获取到Ancs时间 Date
 */
void Cb_BleOnAncsGetDate(u8 *pData, u16 Len);

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_PLAYER中 的 AMS_PLAYATTRIBUTE_ID_PLAYBACK_INFO 
        (PlaybackStatePaused=0 PlaybackStatePlaying=1 PlaybackStateRewinding=2 PlaybackStateFastForwarding=3)
 */
void Cb_BleOnAmsGetPlayState(u8 *pData, u16 Len);

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_PLAYER中 的  的AMS_PLAYATTRIBUTE_ID_VOLUME      
 */
void Cb_BleOnAmsGetVolume(u8 *pData, u16 Len);

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_TRACK中 的  的AMS_TRACK_ATTRIBUTE_ID_ARTIST     
 */
void Cb_BleOnAmsGetArtist(u8 *pData, u16 Len);

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_TRACK中 的  的AMS_TRACK_ATTRIBUTE_ID_TITLE   
 */
void Cb_BleOnAmsGetTitle(u8 *pData, u16 Len);

/**
 * @brief 获取到Ams：AMS_ENTITY_ID_TRACK中 的  的AMS_TRACK_ATTRIBUTE_ID_DURATION   
 */
void Cb_BleOnAmsGetDuration(u8 *pData, u16 Len);

/**
* @brief (resource) 执行蓝牙操作事件
* @return  false:没有用户调度OPS接口;            true:有用户调度OPS接口.
*/
bool System_Ble_Task_OpsDispatcher (void);

/**
* @brief (resource) 唤醒Ble Task执行 
*/
void System_Ble_Task_WakeUp (void);


#ifdef __cplusplus
}
#endif

#endif                                                                  
/**********************************************************************************************************
  END FILE
**********************************************************************************************************/
