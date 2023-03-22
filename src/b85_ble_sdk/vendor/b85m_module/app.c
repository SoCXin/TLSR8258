/********************************************************************************************************
 * @file	app.c
 *
 * @brief	This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app.h"
#include "app_att.h"
#include "spp.h"
#include "battery_check.h"

//module spp Tx / Rx fifo
//MYFIFO_INIT(spp_rx_fifo, 72, 2);
//MYFIFO_INIT(spp_tx_fifo, 72, 8);


_attribute_data_retention_  u8 		 	spp_rx_fifo_b[SPP_RXFIFO_SIZE * SPP_RXFIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	spp_rx_fifo = {
												SPP_RXFIFO_SIZE,
												SPP_RXFIFO_NUM,
												0,
												0,
												spp_rx_fifo_b,};

_attribute_data_retention_  u8 		 	spp_tx_fifo_b[SPP_TXFIFO_SIZE * SPP_TXFIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	spp_tx_fifo = {
												SPP_TXFIFO_SIZE,
												SPP_TXFIFO_NUM,
												0,
												0,
												spp_tx_fifo_b,};



//RF Tx / Rx fifo
//MYFIFO_INIT(blt_rxfifo, 64, 8);
//MYFIFO_INIT(blt_txfifo, 40, 16);


#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		16



_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};


_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_txfifo = {
												TX_FIFO_SIZE,
												TX_FIFO_NUM,
												0,
												0,
												blt_txfifo_b,};















#define     	MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_37
#define 	   	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	   	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_40MS

#define 	   	MY_DIRECT_ADV_TMIE						2000000  //us

#define	 	MY_RF_POWER_INDEX						RF_POWER_P0dBm

#define		BLE_DEVICE_ADDRESS_TYPE 			BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_	own_addr_type_t 	app_own_address_type = OWN_ADDRESS_PUBLIC;


_attribute_data_retention_ u8  g_mac_public[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
_attribute_data_retention_ u8  g_ble_name[30] = {0x00};
_attribute_data_retention_ u8  g_ble_name_len = 0;
_attribute_data_retention_ u8  g_ble_advData[30] = {0x00};
_attribute_data_retention_ u8  g_ble_advData_len = 0;
_attribute_data_retention_ u8  g_ble_scanData[30] = {0x00};
_attribute_data_retention_ u8  g_ble_scanData_len = 0;
_attribute_data_retention_ u32 g_ble_pincode = 0;

/**
 * @brief	Adv Packet data
 */
const u8	tbl_advData[] = {
	 0x05, 0x09, 'k', 'M', 'o', 'd',
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
};
/**
 * @brief	Scan Response Packet data
 */
const u8	tbl_scanRsp [] = {
		 0x07, 0x09, 'k', 'M', 'o', 'd', 'u', 'l',	//scan name " tmodul"
	};


_attribute_data_retention_	u8 	ui_ota_is_working = 0;

_attribute_data_retention_	u32	lowBattDet_tick   = 0;



_attribute_data_retention_ u8 conn_update_cnt;

/**
 * @brief		call back function of connect parameter update response
 * @param[in]	id - connect id
 * @param[in]	result - connect parameter update response result
 * @return      0    - success
 *              else - reserved
 */
int app_conn_param_update_response(u8 id, u16  result)
{
#if 0
	if(result == CONN_PARAM_UPDATE_ACCEPT){
		printf("SIG: the LE master Host has accepted the connection parameters.\n");
		conn_update_cnt = 0;
	}
	else if(result == CONN_PARAM_UPDATE_REJECT)
	{
		printf("SIG: the LE master Host has rejected the connection parameters..\n");
		printf("Current Connection interval:%dus.\n", bls_ll_getConnectionInterval() * 1250 );
		conn_update_cnt++;
		if(conn_update_cnt < 4){
			printf("Slave sent update connPara req!\n");
		}
		if(conn_update_cnt == 1){
			bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_20MS, 0, CONN_TIMEOUT_4S); //18.75ms iOS
		}
		else if(conn_update_cnt == 2){
			bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_20MS, CONN_INTERVAL_40MS, 0, CONN_TIMEOUT_4S);
		}
		else if(conn_update_cnt == 3){
			bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_40MS, CONN_INTERVAL_75MS, 0, CONN_TIMEOUT_4S);
		}
		else{
			conn_update_cnt = 0;
			printf("Slave Connection Parameters Update table all tested and failed!\n");
		}
	}
#endif

	return 0;
}



#if (BLE_OTA_ENABLE)

/**
 * @brief		callback function of ota start
 * @param[in]	none
 * @return      none
 */
void entry_ota_mode(void)
{
	bls_ota_setTimeout(15 * 1000 * 1000); //set OTA timeout  15 seconds

	#if(BLT_APP_LED_ENABLE)
		gpio_set_output_en(GPIO_LED, 1);  //output enable
		gpio_write(GPIO_LED, 1);  //LED on for indicate OTA mode
	#endif
}

/**
 * @brief		callback function of ota result
 * @param[in]	none
 * @return      none
 */
void show_ota_result(int result)
{

	#if(1 && BLT_APP_LED_ENABLE)
		gpio_set_output_en(GPIO_LED, 1);

		if(result == OTA_SUCCESS){  //OTA success
			gpio_write(GPIO_LED, 1);
			sleep_us(500000);
			gpio_write(GPIO_LED, 0);
			sleep_us(500000);
			gpio_write(GPIO_LED, 1);
			sleep_us(500000);
			gpio_write(GPIO_LED, 0);
			sleep_us(500000);
		}
		else{  //OTA fail

			#if 0 //this is only for debug,  can not use this in application code
				irq_disable();
				WATCHDOG_DISABLE;


				while(1){
					gpio_write(GPIO_LED, 1);
					sleep_us(200000);
					gpio_write(GPIO_LED, 0);
					sleep_us(200000);
				}

			#endif

		}

		gpio_set_output_en(GPIO_LED, 0);
	#endif
}
#endif


#define 		MAX_INTERVAL_VAL					16






_attribute_data_retention_	u32 tick_wakeup;
_attribute_data_retention_	int	mcu_uart_working;
_attribute_data_retention_	int	module_uart_working;
_attribute_data_retention_	int module_task_busy;


_attribute_data_retention_	int	module_uart_data_flg;
_attribute_data_retention_	u32 module_wakeup_module_tick;

#define UART_TX_BUSY			( (spp_tx_fifo.rptr != spp_tx_fifo.wptr) || uart_tx_is_busy() )
#define UART_RX_BUSY			(spp_rx_fifo.rptr != spp_rx_fifo.wptr)

/**
 * @brief		obtain uart working status
 * @param[in]	none
 * @return      0 for idle  else for busy
 */
int app_module_busy ()
{
	mcu_uart_working = gpio_read(GPIO_WAKEUP_MODULE);  //mcu use GPIO_WAKEUP_MODULE to indicate the UART data transmission or receiving state
	module_uart_working = UART_TX_BUSY || UART_RX_BUSY; //module checks to see if UART rx and tX are all processed
	module_task_busy = mcu_uart_working || module_uart_working;
	return module_task_busy;
}

/**
 * @brief		exit suspend mode
 * @param[in]	none
 * @return      none
 */
void app_suspend_exit ()
{
	GPIO_WAKEUP_MODULE_HIGH;  //module enter working state
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
	tick_wakeup = clock_time () | 1;
	// printf("app_suspend_exit\n");
}

/**
 * @brief		enter suspend mode
 * @param[in]	none
 * @return      0 - forbidden enter suspend mode
 *              1 - allow enter suspend mode
 */
int app_suspend_enter ()
{
	// printf("app_suspend_enter\n");
	if (app_module_busy ())
	{
		app_suspend_exit ();
		return 0;
	}
	return 1;
}

/**
 * @brief      power management code for application
 * @param[in]  none
 * @return     none
 */
void app_power_management ()
{

#if (BLE_MODULE_INDICATE_DATA_TO_MCU)
	module_uart_working = UART_TX_BUSY || UART_RX_BUSY;

	//When module's UART data is sent, the GPIO_WAKEUP_MCU is lowered or suspended (depending on how the user is designed)
	if(module_uart_data_flg && !module_uart_working){
		module_uart_data_flg = 0;
		module_wakeup_module_tick = 0;
		//GPIO_WAKEUP_MCU_LOW;
		GPIO_WAKEUP_MCU_HIGH;
        GPIO_WAKEUP_IND_HIGH; // indication module wakeup
	}
#endif

	// pullup GPIO_WAKEUP_MODULE: exit from suspend
	// pulldown GPIO_WAKEUP_MODULE: enter suspend

#if (BLE_MODULE_PM_ENABLE)

	if (!app_module_busy() && !tick_wakeup)
	{
		#if (PM_DEEPSLEEP_RETENTION_ENABLE)
			blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
			bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		#else
			bls_pm_setSuspendMask(SUSPEND_ADV | SUSPEND_CONN);
		#endif

		bls_pm_setWakeupSource(PM_WAKEUP_PAD);  // GPIO_WAKEUP_MODULE needs to be wakened
	}

	if (tick_wakeup && clock_time_exceed (tick_wakeup, 500))
	{
		GPIO_WAKEUP_MODULE_LOW;
		tick_wakeup = 0;
	}

#endif
}

typedef union {
 struct{
  u8 bondingFlag : 2;
  u8 MITM : 1;
  u8 SC : 1;
  u8 keyPress: 1;
  u8 rsvd: 3;
 };
 u8 authType;
}smp_authReq_t;

typedef union{
 struct {
  u8 encKey  : 1;
  u8 idKey   : 1;
  u8 sign    : 1;
  u8 linkKey : 4;
 };
 u8 keyIni;
}smp_keyDistribution_t;

typedef struct{
 u8 code;      //req = 0x01; rsp = 0x02;
 u8 ioCapablity;
 u8 oobDataFlag;
 smp_authReq_t authReq;
 u8 maxEncrySize;

 smp_keyDistribution_t initKeyDistribution;
 smp_keyDistribution_t rspKeyDistribution;
}smp_paring_req_rsp_t;

typedef struct{
 u8      paring_tk[16]; // in security connection to keep own random
 u8      own_ltk[16];   //used for generate ediv and random

 smp_paring_req_rsp_t   paring_req;
 smp_paring_req_rsp_t   paring_rsp;
 smp_authReq_t   auth_req;
 u8      own_conn_type;  //current connection peer own type

 u16      rsvd;
 u8      own_conn_addr[6];
 u8                  local_irk[16];
}smp_param_own_t;

extern smp_param_own_t  smp_param_own;

int  blc_smp_setTK (u32 pinCodeInput)
{
    memset(smp_param_own.paring_tk, 0, 16);
    if(pinCodeInput <= 999999){ //0~999999
        memcpy(smp_param_own.paring_tk, &pinCodeInput, 4);

        return 1;
    }

    return 0;
}

void ble_stack_init(void)
{
#if 0
////////////////// BLE stack initialization ////////////////////////////////////
    u8  mac_public[6];
    u8  mac_random_static[6];
    //for 512K Flash, flash_sector_mac_address equals to 0x76000
    //for 1M  Flash, flash_sector_mac_address equals to 0xFF000
	u8 ff_six_byte[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	if ( memcmp(g_mac_public, ff_six_byte, 6) ) 
    {
        blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
        memcpy(mac_public, g_mac_public, 6);
	}
	else
    {
        blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
    }

#if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
        app_own_address_type = OWN_ADDRESS_PUBLIC;
#elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
        app_own_address_type = OWN_ADDRESS_RANDOM;
        blc_ll_setRandomAddr(mac_random_static);
#endif

    ////// Controller Initialization  //////////
    blc_ll_initBasicMCU();                      //mandatory
    blc_ll_initStandby_module(mac_public);              //mandatory
    blc_ll_initAdvertising_module(mac_public);  //adv module:        mandatory for BLE slave,
    blc_ll_initConnection_module();             //connection module  mandatory for BLE slave/master
    blc_ll_initSlaveRole_module();              //slave module:      mandatory for BLE slave,




    ////// Host Initialization  //////////
    blc_gap_peripheral_init();    //gap initialization
    my_att_init (); //gatt initialization
    blc_l2cap_register_handler (blc_l2cap_packet_receive);      //l2cap initialization
    blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);         //register sig process handler

    //Smp Initialization may involve flash write/erase(when one sector stores too much information,
    //   is about to exceed the sector threshold, this sector must be erased, and all useful information
    //   should re_stored) , so it must be done after battery check
#if (BLE_SECURITY_ENABLE)
        //defalut smp4.0, just work
        blc_smp_peripheral_init();

	#if (0) //default close
            //host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
            blc_gap_registerHostEventHandler( app_host_event_callback );
            blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN            |  \
                                  GAP_EVT_MASK_SMP_PARING_SUCCESS           |  \
                                  GAP_EVT_MASK_SMP_PARING_FAIL              |  \
                                  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );
	#endif
#else
        blc_smp_setSecurityLevel(No_Security);
#endif


#endif
    ///////////////////// USER application initialization ///////////////////

    bls_ll_setAdvData( (u8 *)g_ble_advData, g_ble_advData_len);
    bls_ll_setScanRspData( (u8 *)g_ble_scanData, g_ble_scanData_len);
    bls_att_setDeviceName( (u8 *)g_ble_name, g_ble_name_len);



    ////////////////// config adv packet /////////////////////
    u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                                 0,  NULL,
                                 MY_APP_ADV_CHANNEL,
                                 ADV_FP_NONE);

    if(status != BLE_SUCCESS) {
        while(1);
    }  //debug: adv setting err


    bls_ll_setAdvEnable(1);  //adv enable
    rf_set_power_level_index (MY_RF_POWER_INDEX);

    return;
}

/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */
void user_init_normal(void)
{

	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
	random_generator_init();  //this is must

	/*****************************************************************************************
	 Note: battery check must do before any flash write/erase operation, cause flash write/erase
		   under a low or unstable power supply will lead to error flash operation

		   Some module initialization may involve flash write/erase, include: OTA initialization,
				SMP initialization, ..
				So these initialization must be done after  battery check
	*****************************************************************************************/
	#if (BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation
		u8 battery_check_returnVaule = 0;
		if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
			do{
				battery_check_returnVaule = app_battery_power_check(VBAT_ALRAM_THRES_MV + 200);  //2.2 V
			}while(battery_check_returnVaule);
		}
		else{
			do{
				battery_check_returnVaule = app_battery_power_check(VBAT_ALRAM_THRES_MV);  //2.0 V
			}while(battery_check_returnVaule);
		}
	#endif


////////////////// BLE stack initialization ////////////////////////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];
	//for 512K Flash, flash_sector_mac_address equals to 0x76000
	//for 1M  Flash, flash_sector_mac_address equals to 0xFF000
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

	#if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
		app_own_address_type = OWN_ADDRESS_PUBLIC;
	#elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
		app_own_address_type = OWN_ADDRESS_RANDOM;
		blc_ll_setRandomAddr(mac_random_static);
	#endif

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initAdvertising_module(mac_public); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,

    memcpy(g_mac_public, mac_public, 6);


	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	my_att_init (); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization
	blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);         //register sig process handler

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	#if (BLE_SECURITY_ENABLE)
    	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

    	//set security level: "LE_Security_Mode_1_Level_3"
    	blc_smp_setSecurityLevel(Authenticated_Paring_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Paring_with_Encryption)
    	blc_smp_enableAuthMITM(1);
    	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
    	blc_smp_setIoCapability(IO_CAPABILITY_DISPLAY_ONLY);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

		//defalut smp4.0, just work
		blc_smp_peripheral_init();

        blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

		#if (1) //default close
			//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
			blc_gap_registerHostEventHandler( app_host_event_callback );
			blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
								  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
								  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
								  GAP_EVT_MASK_SMP_TK_DISPALY 				|  \
								  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );
		#endif
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif


#if 0
	///////////////////// USER application initialization ///////////////////

	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));



	////////////////// config adv packet /////////////////////
	u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
								 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
								 0,  NULL,
								 MY_APP_ADV_CHANNEL,
								 ADV_FP_NONE);

	if(status != BLE_SUCCESS) {
		while(1);
	}  //debug: adv setting err


	bls_ll_setAdvEnable(1);  //adv enable
	rf_set_power_level_index (MY_RF_POWER_INDEX);
#endif





	////////////////// SPP initialization ///////////////////////////////////
	//note: dma addr must be set first before any other uart initialization!
	u8 *uart_rx_addr = (spp_rx_fifo_b + (spp_rx_fifo.wptr & (spp_rx_fifo.num-1)) * spp_rx_fifo.size);
	uart_recbuff_init( (unsigned char *)uart_rx_addr, spp_rx_fifo.size);

	//uart_gpio_set(UART_TX_PB1, UART_RX_PB0);
    uart_gpio_set(UART_TX_PD7, UART_RX_PA0);

	uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

	//baud rate: 115200
	#if (CLOCK_SYS_CLOCK_HZ == 16000000)
		uart_init(9, 13, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
		uart_init(12, 15, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
		uart_init(30, 8, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
		uart_init(25, 15, PARITY_NONE, STOP_BIT_ONE);
	#endif

	uart_dma_enable(1, 1); 	//uart data in hardware buffer moved by dma, so we need enable them first

	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);   	//uart Rx/Tx dma irq enable

	uart_irq_enable(0, 0);  	//uart Rx/Tx irq no need, disable them

	extern int rx_from_uart_cb (void);
	extern int tx_to_uart_cb (void);
	blc_register_hci_handler(rx_from_uart_cb, tx_to_uart_cb);				//customized uart handler


	extern int controller_event_handler(u32 h, u8 *para, int n);
	blc_hci_registerControllerEventHandler(controller_event_handler);		//register event callback
	bls_hci_mod_setEventMask_cmd(0xfffff);			//enable all 18 events,event list see ll.h





#if (BLE_MODULE_PM_ENABLE)
	blc_ll_initPowerManagement_module();        //pm module:      	 optional

	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
		blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(95, 95);
		blc_pm_setDeepsleepRetentionEarlyWakeupTiming(250);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif


	//mcu can wake up module from suspend or deepsleep by pulling up GPIO_WAKEUP_MODULE
	cpu_set_gpio_wakeup (GPIO_WAKEUP_MODULE, Level_High, 1);  // pad high wakeup deepsleep

	GPIO_WAKEUP_MODULE_LOW;

	bls_pm_registerFuncBeforeSuspend( &app_suspend_enter );
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif



#if (BLE_OTA_ENABLE)
	// OTA init
	bls_ota_clearNewFwDataArea(); //must
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(show_ota_result);
#endif

}


#if (PM_DEEPSLEEP_RETENTION_ENABLE)
	/**
	 * @brief		user initialization when MCU wake_up from deepSleep_retention mode
	 * @param[in]	none
	 * @return      none
	 */
	_attribute_ram_code_ void user_init_deepRetn(void)
	{
		blc_ll_initBasicMCU();   //mandatory
		rf_set_power_level_index (MY_RF_POWER_INDEX);

		blc_ll_recoverDeepRetention();

		DBG_CHN0_HIGH;    //debug

		irq_enable();


		////////////////// SPP initialization ///////////////////////////////////
		//note: dma addr must be set first before any other uart initialization!

		u8 *uart_rx_addr = (spp_rx_fifo_b + (spp_rx_fifo.wptr & (spp_rx_fifo.num-1)) * spp_rx_fifo.size);
		uart_recbuff_init( (unsigned char *)uart_rx_addr, spp_rx_fifo.size);


		//uart_gpio_set(UART_TX_PB1, UART_RX_PB0);
		uart_gpio_set(UART_TX_PD7, UART_RX_PA0);

		uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset


		DBG_CHN0_LOW;  //debug

		//baud rate: 115200
		#if (CLOCK_SYS_CLOCK_HZ == 16000000)
			uart_init(9, 13, PARITY_NONE, STOP_BIT_ONE);
		#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
			uart_init(12, 15, PARITY_NONE, STOP_BIT_ONE);
		#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
			uart_init(30, 8, PARITY_NONE, STOP_BIT_ONE);
		#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
			uart_init(25, 15, PARITY_NONE, STOP_BIT_ONE);
		#endif

		uart_dma_enable(1, 1); 	//uart data in hardware buffer moved by dma, so we need enable them first

		irq_set_mask(FLD_IRQ_DMA_EN);
		dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);   	//uart Rx/Tx dma irq enable

		uart_irq_enable(0, 0);  	//uart Rx/Tx irq no need, disable them

		//mcu can wake up module from suspend or deepsleep by pulling up GPIO_WAKEUP_MODULE
		cpu_set_gpio_wakeup (GPIO_WAKEUP_MODULE, Level_High, 1);  // pad high wakeup deepsleep

		GPIO_WAKEUP_MODULE_LOW;




		DBG_CHN0_HIGH;   //debug
	}
#endif

/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop (void)
{
	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();

	////////////////////////////////////// UI entry /////////////////////////////////

#if (BATT_CHECK_ENABLE)
	if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
		lowBattDet_tick = clock_time();
		if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
			app_battery_power_check(VBAT_ALRAM_THRES_MV + 200);  //2.2 V
		}
		else{
			app_battery_power_check(VBAT_ALRAM_THRES_MV);  //2.0 V
		}
	}
#endif


	//  add spp UI task
	// app_power_management ();


	// spp_restart_proc();
}
