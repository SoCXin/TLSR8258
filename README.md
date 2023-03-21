# [TLSR8258](https://doc.soc.xin/TLSR8258)

* [Telink](https://www.telink-semi.com/): [RISC](https://github.com/SoCXin/RISC-V)
* [L1R1](https://github.com/SoCXin/Level): 48 MHz

## [简介](https://github.com/SoCXin/TLSR8258/wiki)

[TLSR8258](http://wiki.telink-semi.cn/wiki/chip-series/TLSR825x-Series/) supports BLE 5.0 with 2X data rate, 4X long range (LR), 8X advertising packet extension (AE), up to 8 antennas for Indoor Positioning, both Telink proprietary and SIG BLE Mesh, Zigbee, RF4CE, HomeKit, Thread, ANT and 2.4GHz proprietary.

### 关键参数

* 48MHz RISC Core
* 48kB SRAM + 512kB Flash
* 14bit 6-channel SAR ADC, 4-channel PGA,effective bits: 10.5bits
* USB + SPI + USART + IIC
* BLE 5.0 TX mode: 4.8mA @ 0dBm with DCDC
* Deep sleep with external wakeup (without SRAM retention): 0.4µA

## [资源收录](https://github.com/SoCXin)

* [参考资源](src/)
* [参考文档](docs/)
* [参考工程](project/)

## [选型建议](https://github.com/SoCXin)

[官方SDK](http://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/)

[TLSR8258](https://github.com/SoCXin/TLSR8258) 相对TLSR8253、TLSR8251支持更多的协议，拥有更大的RAM，性价比最高的 [TLSR8251](https://github.com/SoCXin/TLSR8258)是小米米家温湿度传感器的主控芯片。

TLSR827x系列继承了TLSR825x对多协议的支持，并做了广泛的性能优化，包括支持Bluetooth LE 5.1协议的角度寻向功能，集成RF电感减少BOM，增强音频功能，更宽的供电电压范围，预留Wi-Fi PTA硬件接口，并进一步提升DCDC效率从而降低了系统功耗。

TLSR835x系列则配置了USB外设