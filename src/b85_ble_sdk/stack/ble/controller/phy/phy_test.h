/********************************************************************************************************
 * @file	phy_test.h
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
#ifndef PHY_TEST_H_
#define PHY_TEST_H_

#include "tl_common.h"



/**
 * @brief	PHY test mode
 */
#ifndef			PHYTEST_MODE_DISABLE
#define			PHYTEST_MODE_DISABLE					0
#endif

#ifndef			PHYTEST_MODE_THROUGH_2_WIRE_UART
#define 		PHYTEST_MODE_THROUGH_2_WIRE_UART		1   //Direct Test Mode through a 2-wire UART interface
#endif

#ifndef			PHYTEST_MODE_OVER_HCI_WITH_USB
#define 		PHYTEST_MODE_OVER_HCI_WITH_USB			2   //Direct Test Mode over HCI(UART hardware interface)
#endif

#ifndef			PHYTEST_MODE_OVER_HCI_WITH_UART
#define 		PHYTEST_MODE_OVER_HCI_WITH_UART			3   //Direct Test Mode over HCI(USB  hardware interface)
#endif




/**
 * @brief	PHY test mode enableor disable
 */
#define 		BLC_PHYTEST_DISABLE						0
#define 		BLC_PHYTEST_ENABLE						1









/**
 * @brief      for user to initialize PHY test module
 * @param      none
 * @return     none
 */
void 	  blc_phy_initPhyTest_module(void);


/**
 * @brief      for user to set PHY test enable or disable
 * @param[in]  en - 1: enable; 0:disable
 * @return     status: 0x00 command OK, no other rvalue
 */
ble_sts_t blc_phy_setPhyTestEnable (u8 en);


/**
 * @brief      for user to get PHY test status: enable or disable
 * @param      none
 * @return     1: PHY test is enable; 0: PHY test is disable
 */
bool 	  blc_phy_isPhyTestEnable(void);


#if (MCU_CORE_TYPE == MCU_CORE_9518)
	/**
	 * @brief      uart RX data process for PHY test 2 wire UART mode
	 * @param      none
	 * @return     always 0
	 */
	int 	 blc_phyTest_2wire_rxUartCb (void);


	/**
	 * @brief      uart TX data process for PHY test 2 wire UART mode
	 * @param      none
	 * @return     always 0
	 */
	int 	 blc_phyTest_2wire_txUartCb (void);


	/**
	 * @brief      uart RX data process for PHY test hci UART mode
	 * @param      none
	 * @return     always 0
	 */
	int blc_phyTest_hci_rxUartCb (void);
#elif (MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
	/**
	 * @brief      uart RX data process for PHY test 2 wire UART mode
	 * @param      none
	 * @return     always 0
	 */
	int 	 phy_test_2_wire_rx_from_uart (void);


	/**
	 * @brief      uart TX data process for PHY test 2 wire UART mode
	 * @param      none
	 * @return     always 0
	 */
	int 	 phy_test_2_wire_tx_to_uart (void);

	/**
	 * @brief      This function is used to start a test where the DUT receives test reference
	 *				packets at a fixed interval. The tester generates the test reference packets.
	 * @param[in]     rx_chn  	specify the RF channel to be used by the receiver
	 * @param[in]	  phy_mode 	specify the RF PHT to be used by the receiver
	 * @param[in]	  modulation_index  specifies whether or not the Controller should assume the receiver has a stable modulation index
	 * @return     ble_sts_t
	 */
	ble_sts_t blc_phy_setEnhancedReceiverTest (u8 rx_chn,u8 phy_mode,u8 modulation_index);

	/**
	 * @brief       This function is used to start a test where the DUT generates test reference packets at a fixed interval
	 * @param[in]     tx_chn  	specify the RF channel to be used by the transmitter
	 * @param[in]	  length 	Length in bytes of payload data in each packet
	 * @param[in]	  pkt_type  specify contents of the payload of the test reference packets
	 * @param[in]	  phy_mode  specify the RF PHT to be used by the transmitter
	 * @return       ble_sts_t
	 */
	ble_sts_t blc_phy_setEnhancedTransmitterTest (u8 tx_chn, u8 length, u8 pkt_type,u8 phy_mode);
#endif




#endif /* PHY_TEST_H_ */
