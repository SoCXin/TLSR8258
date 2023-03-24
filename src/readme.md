## 相关示例

* b85m_ble_sample 模拟键盘输入设备，可以实现音量调节

用于在自己的模组上实现相应的功能。当前自己的模组还没有适配EC616L和开发部，需要调试UART和相应的管脚。

* 24MHz晶振
* BLE-NB-UART2-RX PD7
* BLE-NB-UART2-TX PB7
* BLE-UART-RX PC3
* BLE-UART-TX PC2
* WAKEUP PB6
* BLE_GPIO PD2

## Master

B85m master kma dongle 是 BLE master single connection 的 demo，可以和 B85m ble sample/B85m ble remote/B85m module 连接并通信。

发现协议是通过RSSI来进行判断的
```
    if(blm_push_fifo(BLM_CONN_HANDLE, dat)){}
```

scan 回调函数
```
    int blm_le_adv_report_event_handle(u8 *p)
```

### module

针对底板模组的BLE开发计划，用于新方案实现和测试闭环

测试相关功能接口：指令串口和debug串口无法在BLE运行中同时使用，表现在指令串口数据没有响应，但是系统在运行。

* 需要关闭PM

set advertising data: 02 ff 06 00 01 02 03 04 05 06

blc_gatt_pushHandleValueNotify 用于发送数据


从机功能
```
	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);	    //mandatory
	blc_ll_initAdvertising_module(mac_public); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,
	blc_ll_initPowerManagement_module();        //pm module:      	 optional

```

主机功能
```
	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();
	blc_ll_initStandby_module(mac_public);	    //mandatory
	blc_ll_initScanning_module(mac_public); 	//scan module: 		 mandatory for BLE master,
	blc_ll_initInitiating_module();			    //initiate module: 	 mandatory for BLE master,
	blc_ll_initConnection_module();						//connection module  mandatory for BLE slave/master
	blc_ll_initMasterRoleSingleConn_module();			//master module: 	 mandatory for BLE master,

```

```
	//set scan parameter and scan enable
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,	\
							OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);
```

```
    u8 mac_public[6] = {……};
    blc_ll_initBasicMCU();
    blc_ll_initStandby_module(tbl_mac);
    blc_ll_initScanning_module( tbl_mac);
```


```
    if(vc_event.cnt == 2)  //two key press
    {
        u8 dat[32]={0};
        u8 * data = "Warning";

        #if (BLE_HOST_SIMPLE_SDP_ENABLE)
        att_req_write_cmd(dat, 0x15, data, strlen(data));
        #else
        att_req_write_cmd(dat, 25, data, strlen(data));
        #endif
        
        if(blm_push_fifo(BLM_CONN_HANDLE, dat)){
        
        }
    }
        
```