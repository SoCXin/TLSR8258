/********************************************************************************************************
 * @file	blm_pair.c
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
#include "blm_att.h"
#include "blm_pair.h"
#include "blm_host.h"


/**********************************************************************************
				// proc user  PAIR and UNPAIR
**********************************************************************************/
#if (!BLE_HOST_SMP_ENABLE)


man_pair_t blm_manPair;

/* define pair slave max num,
   if exceed this max num, two methods to process new slave pairing
   method 1: overwrite the oldest one(telink use this method)
   method 2: not allow paring unness unpair happened  */
#define	USER_PAIR_SLAVE_MAX_NUM       1  //telink use max 1


typedef struct {
	u8 bond_mark;
	u8 adr_type;
	u8 address[6];
} macAddr_t;


typedef struct {
	u32 bond_flash_idx[USER_PAIR_SLAVE_MAX_NUM];  //mark paired slave mac address in flash
	macAddr_t bond_device[USER_PAIR_SLAVE_MAX_NUM];  //macAddr_t alreay defined in ble stack
	u8 curNum;
} user_salveMac_t;



//current connect slave mac adr: when dongle establish conn with slave, it will record the device mac adr
//when unpair happens, you can select this addr to delete from  slave mac adr table.
extern u8 slaveMac_curConnect[6];   //already defined in ble stack

/* flash erase strategy:
   never erase flash when dongle is working, for flash sector erase takes too much time(20-100 ms)
   this will lead to timing err
   so we only erase flash at initiation,  and with mark 0x00 for no use symbol
 */

#define ADR_BOND_MARK 		0x5A
#define ADR_ERASE_MARK		0x00
/* flash stored mac address struct:
   every 8 bytes is a address area: first one is mark, second no use, third - eighth is 6 byte address
   	   0     1           2 - 7
   | mark |     |    mac_address     |
   mark = 0xff, current area is invalid, pair info end
   mark = 0x01, current area is valid, load the following mac_address,
   mark = 0x00, current area is invalid (previous valid address is erased)
 */

int		user_bond_slave_flash_cfg_idx;  //new mac address stored flash idx


user_salveMac_t user_tbl_slaveMac;  //slave mac bond table

/**
 * @brief		this function serves to remove the oldest adr in slave mac table
 * @param[in]	index
 * @return      none
 */
void user_tbl_slave_mac_delete_by_index(int index)  //remove the oldest adr in slave mac table
{
	//erase the oldest with ERASE_MARK
	u8 delete_mark = ADR_ERASE_MARK;
	flash_write_page (FLASH_ADR_PARING + user_tbl_slaveMac.bond_flash_idx[index], 1, &delete_mark);

	for(int i=index; i<user_tbl_slaveMac.curNum - 1; i++){ 	//move data
		user_tbl_slaveMac.bond_flash_idx[i] = user_tbl_slaveMac.bond_flash_idx[i+1];
		memcpy( (u8 *)&user_tbl_slaveMac.bond_device[i], (u8 *)&user_tbl_slaveMac.bond_device[i+1], 8 );
	}

	user_tbl_slaveMac.curNum --;
}

/**
 * @brief		this function serves to add new mac address to table
 * @param[in]	adr_type
 * @param[in]	adr - pointer to point the address data
 * @return      none
 */
int user_tbl_slave_mac_add(u8 adr_type, u8 *adr)  //add new mac address to table
{
	u8 add_new = 0;
	if(user_tbl_slaveMac.curNum >= USER_PAIR_SLAVE_MAX_NUM){ //salve mac table is full
		//slave mac max, telink use  method 1: overwrite the oldest one
		user_tbl_slave_mac_delete_by_index(0);  //overwrite, delete index 0 (oldest) of table
		add_new = 1;  //add new
	}
	else{//slave mac table not full
		add_new = 1;
	}

	if(add_new){

		user_tbl_slaveMac.bond_device[user_tbl_slaveMac.curNum].bond_mark = ADR_BOND_MARK;
		user_tbl_slaveMac.bond_device[user_tbl_slaveMac.curNum].adr_type = adr_type;
		memcpy(user_tbl_slaveMac.bond_device[user_tbl_slaveMac.curNum].address, adr, 6);

		user_bond_slave_flash_cfg_idx += 8;  //inc flash idx to get the new 8 bytes area
		flash_write_page (FLASH_ADR_PARING + user_bond_slave_flash_cfg_idx, 8, (u8 *)&user_tbl_slaveMac.bond_device[user_tbl_slaveMac.curNum] );

		user_tbl_slaveMac.bond_flash_idx[user_tbl_slaveMac.curNum] = user_bond_slave_flash_cfg_idx;  //mark flash idx
		user_tbl_slaveMac.curNum++;

		return 1;  //add OK
	}

	return 0;
}

/**
 * @brief		this function serves to search mac address in the bond slave mac table:
 *              when slave paired with dongle, add this addr to table
 * 				re_poweron slave, dongle will search if this AdvA in slave adv pkt is in this table
 * 				if in, it will connect slave directly
 * 				this function must in ramcode
 * @param[in]	adr_type
 * @param[in]	adr - pointer to point the address data
 * @return      none
 */
int user_tbl_slave_mac_search(u8 adr_type, u8 * adr)
{
	for(int i=0; i< user_tbl_slaveMac.curNum; i++){
		if( user_tbl_slaveMac.bond_device[i].adr_type == adr_type && \
			!memcmp(user_tbl_slaveMac.bond_device[i].address ,adr, 6)){  //match

			return (i+1);  //return index+1( 1 - USER_PAIR_SLAVE_MAX_NUM)
		}
	}

	return 0;
}

/**
 * @brief		this function serves to remove adr from slave mac table by adr
 * 				when rc trigger unpair, use this function to delete the slave mac
 * @param[in]	adr_type
 * @param[in]	adr - pointer to point the address data
 * @return      none
 */
int user_tbl_slave_mac_delete_by_adr(u8 adr_type, u8 *adr)  //remove adr from slave mac table by adr
{
	for(int i=0;i<user_tbl_slaveMac.curNum;i++){
		if( user_tbl_slaveMac.bond_device[i].adr_type == adr_type && \
			!memcmp(user_tbl_slaveMac.bond_device[i].address ,adr, 6)){  //match

			//erase the match adr
			u8 delete_mark = ADR_ERASE_MARK;
			flash_write_page (FLASH_ADR_PARING + user_tbl_slaveMac.bond_flash_idx[i], 1, &delete_mark);

			for(int j=i; j< user_tbl_slaveMac.curNum - 1;j++){ //move data
				user_tbl_slaveMac.bond_flash_idx[j] = user_tbl_slaveMac.bond_flash_idx[j+1];
				memcpy((u8 *)&user_tbl_slaveMac.bond_device[j], (u8 *)&user_tbl_slaveMac.bond_device[j+1], 8);
			}

			user_tbl_slaveMac.curNum --;
			return 1; //delete OK
		}
	}

	return 0;
}



/**
 * @brief		this function serves to delete all the  adr in slave mac table
 * @param[in]	none
 * @return      none
 */
void user_tbl_slave_mac_delete_all(void)  //delete all the  adr in slave mac table
{
	u8 delete_mark = ADR_ERASE_MARK;
	for(int i=0; i< user_tbl_slaveMac.curNum; i++){
		flash_write_page (FLASH_ADR_PARING + user_tbl_slaveMac.bond_flash_idx[i], 1, &delete_mark);
		memset( (u8 *)&user_tbl_slaveMac.bond_device[i], 0, 8);
		//user_tbl_slaveMac.bond_flash_idx[i] = 0;  //do not  concern
	}

	user_tbl_slaveMac.curNum = 0;
}


/**
 * @brief		this function serves to unpair cmd proc
 * 				when press unpair button on dongle or send unpair cmd from slave
 * 				dongle will call this function to process current unpair cmd
 * @param[in]	none
 * @return      none
 */
void user_tbl_salve_mac_unpair_proc(void)
{
	//telink will delete all adr when unpair happens, you can change to your own strategy
	//slaveMac_curConnect is for you to use
	user_tbl_slave_mac_delete_all();
}

u8 adbg_flash_clean;
#define DBG_FLASH_CLEAN   0

/**
 * @brief		this function serves to erase flash which stored bond slave address
 * 				when flash stored too many addr, it may exceed a sector max(4096), so we must clean this sector
 * 				and rewrite the valid addr at the beginning of the sector(0x11000)
 * @param[in]	none
 * @return      none
 */
void	user_bond_slave_flash_clean (void)
{
#if	DBG_FLASH_CLEAN
	if (user_bond_slave_flash_cfg_idx < 8*8)  //debug, max 8 area, then clean flash
#else
	if (user_bond_slave_flash_cfg_idx < 2048)  //max 2048/8 = 256,rest available sector is big enough, no need clean
#endif
	{
		return;
	}

	adbg_flash_clean = 1;

	flash_erase_sector (FLASH_ADR_PARING);

	user_bond_slave_flash_cfg_idx = -8;  //init value for no bond slave mac

	//rewrite bond table at the beginning of 0x11000
	for(int i=0; i< user_tbl_slaveMac.curNum; i++){
		//u8 add_mark = ADR_BOND_MARK;

		user_bond_slave_flash_cfg_idx += 8;  //inc flash idx to get the new 8 bytes area
		flash_write_page (FLASH_ADR_PARING + user_bond_slave_flash_cfg_idx, 8, (u8*)&user_tbl_slaveMac.bond_device[i] );

		user_tbl_slaveMac.bond_flash_idx[i] = user_bond_slave_flash_cfg_idx;  //update flash idx
	}
}

/**
 * @brief		this function serves to init the master host paring
 * @param[in]	none
 * @return      none
 */
void	user_master_host_pairing_flash_init(void)
{
	u8 *pf = (u8 *)FLASH_ADR_PARING;
	for (user_bond_slave_flash_cfg_idx=0; user_bond_slave_flash_cfg_idx<4096; user_bond_slave_flash_cfg_idx+=8)
	{ //traversing 8 bytes area in sector 0x11000 to find all the valid slave mac adr
		if( pf[user_bond_slave_flash_cfg_idx] == ADR_BOND_MARK ){  //valid adr
			if(user_tbl_slaveMac.curNum < USER_PAIR_SLAVE_MAX_NUM){
				user_tbl_slaveMac.bond_flash_idx[user_tbl_slaveMac.curNum] = user_bond_slave_flash_cfg_idx;
				flash_read_page (FLASH_ADR_PARING + user_bond_slave_flash_cfg_idx, 8, (u8 *)&user_tbl_slaveMac.bond_device[user_tbl_slaveMac.curNum] );
				user_tbl_slaveMac.curNum ++;
			}
			else{ //slave mac in flash more than max, we think it's code bug
				write_reg32(0x9000,0x12345678);  //for debug
				irq_disable();
				while(1);
			}
		}
		else if (pf[user_bond_slave_flash_cfg_idx] == 0xff)	//end
		{
			break;
		}
	}

	user_bond_slave_flash_cfg_idx -= 8; //back to the newest addr 8 bytes area flash ixd(if no valid addr, will be -8)

	user_bond_slave_flash_clean ();
}

/**
 * @brief		this function serves to init the master host paring management
 * @param[in]	none
 * @return      none
 */
void user_master_host_pairing_management_init(void)
{
	user_master_host_pairing_flash_init();
}



#endif  //end of !BLE_HOST_SMP_ENABLE













int	dongle_pairing_enable = 0;
int dongle_unpair_enable = 0;


/**
 * @brief		host pair or upair proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_
void host_pair_unpair_proc(void)
{

#if (!BLE_HOST_SMP_ENABLE)
    if(blm_manPair.manual_pair && clock_time_exceed(blm_manPair.pair_tick, 2000000)){  //@@
    	blm_manPair.manual_pair = 0;
    }
#endif


	//terminate and unpair proc
	static int master_disconnect_flag;
	if(dongle_unpair_enable){
		if(!master_disconnect_flag && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			if( blm_ll_disconnect(cur_conn_device.conn_handle, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){
				master_disconnect_flag = 1;
				dongle_unpair_enable = 0;

				#if (BLE_HOST_SMP_ENABLE)
					tbl_bond_slave_unpair_proc(cur_conn_device.mac_adrType, cur_conn_device.mac_addr); //by telink stack host smp
				#else
					user_tbl_salve_mac_unpair_proc();
				#endif
			}
		}
	}
	if(master_disconnect_flag && blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		master_disconnect_flag = 0;
	}
}










