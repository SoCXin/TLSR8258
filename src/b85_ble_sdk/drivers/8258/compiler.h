/********************************************************************************************************
 * @file	compiler.h
 *
 * @brief	This is the header file for B85
 *
 * @author	Driver Group
 * @date	May 8,2018
 *
 * @par     Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions are met:
 *
 *              1. Redistributions of source code must retain the above copyright
 *              notice, this list of conditions and the following disclaimer.
 *
 *              2. Unless for usage inside a TELINK integrated circuit, redistributions
 *              in binary form must reproduce the above copyright notice, this list of
 *              conditions and the following disclaimer in the documentation and/or other
 *              materials provided with the distribution.
 *
 *              3. Neither the name of TELINK, nor the names of its contributors may be
 *              used to endorse or promote products derived from this software without
 *              specific prior written permission.
 *
 *              4. This software, with or without modification, must only be used with a
 *              TELINK integrated circuit. All other usages are subject to written permission
 *              from TELINK and different commercial license may apply.
 *
 *              5. Licensee shall be solely responsible for any claim to the extent arising out of or
 *              relating to such deletion(s), modification(s) or alteration(s).
 *
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *          DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 *          DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************************************/
#pragma once


#define _attribute_packed_				__attribute__((packed))
#define _attribute_aligned_(s)			__attribute__((aligned(s)))
#define _attribute_session_(s)			__attribute__((section(s)))
#define _attribute_ram_code_  			_attribute_session_(".ram_code")
#define _attribute_ram_code_noinline_	__attribute__((section(".ram_code"))) __attribute__((noinline))
#define _attribute_custom_code_  		_attribute_session_(".custom") volatile
#define _attribute_no_inline_   		__attribute__((noinline))
#define _inline_ 						inline

#define _attribute_ram_code_sec_      		__attribute__((section(".ram_code")))
#define _attribute_ram_code_sec_noinline_	__attribute__((section(".ram_code"))) __attribute__((noinline))

#ifndef	BLC_PM_DEEP_RETENTION_MODE_EN
#define	BLC_PM_DEEP_RETENTION_MODE_EN		1
#endif

#if (BLC_PM_DEEP_RETENTION_MODE_EN)
	#define _attribute_data_retention_   	_attribute_session_(".retention_data")
	#define _attribute_data_no_init_   		_attribute_session_(".data_no_init")    //save retention ram size
#else
    #define _attribute_data_retention_
	#define _attribute_data_no_init_
#endif


