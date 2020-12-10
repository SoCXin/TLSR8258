/********************************************************************************************************
 * @file     app.c 
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     May. 10, 2018
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
#include "app.h"
#include <stack/ble/ble.h>
#include "tl_common.h"
#include "drivers.h"
#include "app_config.h"
#include "vendor/common/blt_led.h"
#include "application/keyboard/keyboard.h"
#include "vendor/common/tl_audio.h"
#include "vendor/common/blt_soft_timer.h"



void user_init_normal(void)
{
#if (INTER_TEST_MODE == TEST_SLAVE_CODED_PHY)
	function_slave_coded_phy_init_normal();
#elif (INTER_TEST_MODE == TEST_MASTER_CODED_PHY)
	function_master_coded_phy_init_normal();
#elif (INTER_TEST_MODE == TEST_2M_MASTER_CONN_MD)
	function_m2M_md_test_init_normal();
#elif (INTER_TEST_MODE == TEST_2M_SLAVE_CONN_MD)
	function_s2M_md_test_init_normal();
#else

#endif
}




_attribute_ram_code_ void user_init_deepRetn(void)
{

#if (INTER_TEST_MODE == TEST_SLAVE_CODED_PHY)
	function_slave_coded_phy_init_deepRetn();
#elif (INTER_TEST_MODE == TEST_MASTER_CODED_PHY)
	function_master_coded_phy_init_deepRetn();
#elif (INTER_TEST_MODE == TEST_2M_MASTER_CONN_MD)
	function_m2M_md_test_init_deepRetn();
#elif (INTER_TEST_MODE == TEST_2M_SLAVE_CONN_MD)
	function_s2M_md_test_init_deepRetn();
#else

#endif
}





u32 tick_loop=0;
/*----------------------------------------------------------------------------*/
/*-------- Main Loop                                                ----------*/
/*----------------------------------------------------------------------------*/
_attribute_ram_code_ void main_loop (void)
{
	tick_loop++;



	blt_sdk_main_loop();

#if (INTER_TEST_MODE == TEST_SLAVE_CODED_PHY)
	function_slave_coded_phy_mainloop();
#elif (INTER_TEST_MODE == TEST_MASTER_CODED_PHY)
	function_master_coded_phy_mainloop();
#elif (INTER_TEST_MODE == TEST_2M_MASTER_CONN_MD)
	function_m2M_md_test_mainloop();
#elif (INTER_TEST_MODE == TEST_2M_SLAVE_CONN_MD)
	function_s2M_md_test_mainloop();
#else

#endif
}

/*----------------------------- End of File ----------------------------------*/


