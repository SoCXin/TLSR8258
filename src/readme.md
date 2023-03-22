## 相关示例

* b85m_ble_sample 模拟键盘输入设备，可以实现音量调节

### module

测试相关功能接口：指令串口和debug串口无法在BLE运行中同时使用，表现在指令串口数据没有响应，但是系统在运行。

set advertising data: 02 ff 06 00 01 02 03 04 05 06

blc_gatt_pushHandleValueNotify 用于发送数据

## V3.4.2.1_Patch_0003

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0003.


### Bug Fixes
* Fix issue when update firmware from version before 3.4.2.0 to 3.4.2.0 and later by OTA, user may fail to reconnect and have to re-pair product.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0003.


### Bug Fixes
* 修复一个问题：当客户程序从3.4.2.0之前的版本通过OTA升级到3.4.2.0及以后，会遇到无法重连，只能重新配对。


### BREAKING CHANGES
* N/A.



## V3.4.2.1_Patch_0002

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0002.


### Bug Fixes
* Fix boundary conditions issue of ATT.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0002.


### Bug Fixes
* 增加处理ATT边界条件下的错误.


### BREAKING CHANGES
* N/A.



## V3.4.2.1_Patch_0001

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0001.


### Bug Fixes
* Add parameter cheack when doing EXCHANGE_MTU_SIZE .


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0001.


### Bug Fixes
* 增加处理EXCHANGE_MTU_SIZE前的参数监测.


### BREAKING CHANGES
* N/A.
