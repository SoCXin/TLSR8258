/*********************************************************************************************

***********
 * @file     ancs.h
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
#ifndef ANCS_H_
#define ANCS_H_
///type

#include "ancsDef.h"

void ancsInit();
void ancsHaveNews ();
void latency_turn_off_once();
void ancsStackCallback(u8 *p);
void ancsFuncSetEnable(u8 enable);
void ancs_handleIncomingCall(u8 accept);
u8 ancsFuncIsEn();
bool ancs_findAncsService();

#endif /* ANCS_H_ */
