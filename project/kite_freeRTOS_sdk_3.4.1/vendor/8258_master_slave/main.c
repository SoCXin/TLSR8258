/********************************************************************************************************
 * @file	 main.c
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     May. 22, 2019
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

extern void user_init();
extern void usb_endpoints_irq_handler (void);
extern void usb_init(void) ;
extern void main_loop (void);



extern void irq_ble_master_slave_handler(void);
_attribute_ram_code_ void irq_handler(void)
{

//	DBG_CHN7_HIGH;

	irq_ble_master_slave_handler ();

//	DBG_CHN7_LOW;


}

int main (void)
{
	cpu_wakeup_init();

#if (CLOCK_SYS_CLOCK_HZ == 32000000)
	clock_init(SYS_CLK_32M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
	clock_init(SYS_CLK_48M_Crystal);
#endif

	gpio_init(1);

	rf_drv_init(RF_MODE_BLE_1M);

#if (APPLICATION_DONGLE)
	usb_init ();
#endif

	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value


#if FIRMWARES_SIGNATURE_ENABLE
	blt_firmware_signature_check();
#endif

	user_init();

    irq_enable();

	while (1) {
#if (MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
#endif
		main_loop ();
	}
}


