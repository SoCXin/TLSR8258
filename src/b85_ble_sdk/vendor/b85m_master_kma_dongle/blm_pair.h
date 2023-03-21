/********************************************************************************************************
 * @file	blm_pair.h
 *
 * @brief	This is the header file for BLE SDK
 *
 * @author	BLE GROUP
 * @date	06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef BLM_PAIR_H_
#define BLM_PAIR_H_

extern int	dongle_pairing_enable;
extern int dongle_unpair_enable;

#if (!BLE_HOST_SMP_ENABLE)

	typedef struct{
		u8 manual_pair;
		u8 mac_type;  //addrsss type
		u8 mac[6];
		u32 pair_tick;
	}man_pair_t;

	extern man_pair_t blm_manPair;


	void user_master_host_pairing_management_init(void);
	void user_tbl_salve_mac_unpair_proc(void);
	int user_tbl_slave_mac_search(u8 adr_type, u8 * adr);
	int user_tbl_slave_mac_add(u8 adr_type, u8 *adr);
	int user_tbl_slave_mac_delete_by_adr(u8 adr_type, u8 *adr);

#endif

/**
 * @brief       this function serves to set current ReadRequest attribute handle
 * @param[in]	handle - connect handle
 * @return      none
 */
void app_setCurrentReadReq_attHandle(u16 handle);

/**
 * @brief		host pair or upair proc in main loop
 * @param[in]	none
 * @return      none
 */
void host_pair_unpair_proc(void);

#endif /* APP_PAIR_H_ */
