/********************************************************************************************************
 * @file     app.h 
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
#ifndef _APP_H
#define _APP_H

#include "tl_common.h"
#include "drivers.h"

void user_init_normal(void);
void user_init_deepRetn(void);
void main_loop (void);


void function_slave_coded_phy_init_normal(void);
void function_slave_coded_phy_init_deepRetn(void);
void function_slave_coded_phy_mainloop(void);


void function_master_coded_phy_init_normal(void);
void function_master_coded_phy_init_deepRetn(void);
void function_master_coded_phy_mainloop(void);


void function_s2M_md_test_init_normal(void);
void function_s2M_md_test_init_deepRetn(void);
void function_s2M_md_test_mainloop(void);


void function_m2M_md_test_init_normal(void);
void function_m2M_md_test_init_deepRetn(void);
void function_m2M_md_test_mainloop(void);

#endif /* APP_H_ */
