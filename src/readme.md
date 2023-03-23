## 相关示例

* b85m_ble_sample 模拟键盘输入设备，可以实现音量调节

用于在自己的模组上实现相应的功能。

### module

针对底板模组的BLE开发计划，用于新方案实现和测试闭环

测试相关功能接口：指令串口和debug串口无法在BLE运行中同时使用，表现在指令串口数据没有响应，但是系统在运行。

* 需要关闭PM

set advertising data: 02 ff 06 00 01 02 03 04 05 06

blc_gatt_pushHandleValueNotify 用于发送数据

