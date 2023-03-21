/********************************************************************************************************
 * @file	uart.h
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
#include "register.h"
#include "gpio.h"
#include "compiler.h"

#ifndef     uart_H
#define     uart_H
#define uart_rtx_pin_tx_trig() uart_clr_tx_done()

/**
 *  @brief  Define mul bits
 */
enum{
	UART_BW_MUL1  = 0,
	UART_BW_MUL2  = 1,
	UART_BW_MUL3  = 2,
	UART_BW_MUL4  = 3,
};

/**
 *  @brief  Define parity type
 */
typedef enum {
    PARITY_NONE = 0,
    PARITY_EVEN,
    PARITY_ODD,
} UART_ParityTypeDef;

/**
 *  @brief  Define the length of stop bit
 */
typedef enum {
    STOP_BIT_ONE          = 0,
    STOP_BIT_ONE_DOT_FIVE = BIT(4),
    STOP_BIT_TWO          = BIT(5),
} UART_StopBitTypeDef;

/**
 *  @brief  Define UART RTS mode
 */
typedef enum {
    UART_RTS_MODE_AUTO = 0,
    UART_RTS_MODE_MANUAL,
} UART_RTSModeTypeDef;

/**
 *  @brief  Define UART TX pin:A2 B1 D0 D3 D7
 */

typedef enum{
	UART_TX_PA2 = GPIO_PA2,
	UART_TX_PB1 = GPIO_PB1,
	UART_TX_PC2 = GPIO_PC2,
	UART_TX_PD0 = GPIO_PD0,
	UART_TX_PD3 = GPIO_PD3,
	UART_TX_PD7 = GPIO_PD7,
}UART_TxPinDef;

/**
 *  @brief  Define UART RX pin: A0 B0 B7 C3 C5 D6
 */

typedef enum{
	UART_RX_PA0 = GPIO_PA0,
	UART_RX_PB0 = GPIO_PB0,
	UART_RX_PB7 = GPIO_PB7,
	UART_RX_PC3 = GPIO_PC3,
	UART_RX_PC5 = GPIO_PC5,
	UART_RX_PD6 = GPIO_PD6,
}UART_RxPinDef;

/**
 *  @brief  Define UART RTX pin: C2 D0 D3 D7
 */
typedef enum{
	UART_RTX_PC2 = GPIO_PC2,
	UART_RTX_PD0 = GPIO_PD0,
	UART_RTX_PD3 = GPIO_PD3,
	UART_RTX_PD7 = GPIO_PD7,
}UART_RTxPinDef;

/**
 *  @brief  Define UART CTS pin : A3 B2 C4 D1
 */

typedef enum{
	UART_CTS_PA3 = GPIO_PA3,
	UART_CTS_PB2 = GPIO_PB2,
	UART_CTS_PC4 = GPIO_PC4,
	UART_CTS_PD1 = GPIO_PD1,
}UART_CtsPinDef;

/**
 *  @brief  Define UART RTS pin : A4  B3 B6 C0
 */

typedef enum{
	UART_RTS_PA4 = GPIO_PA4,
	UART_RTS_PB3 = GPIO_PB3,
	UART_RTS_PB6 = GPIO_PB6,
	UART_RTS_PC0 = GPIO_PC0,
}UART_RtsPinDef;



/**
 * @brief     This function servers to indicate Tx state.
 * @param[in] none.
 * @return    the state of Tx 0:Tx done 1:not.
 */
static inline unsigned char uart_tx_is_busy(void)
{
    return ( (reg_uart_status1 & FLD_UART_TX_DONE) ? 0 : 1) ;
}


/**
 * @brief     This function resets the UART module.
 * @param[in] none
 * @return    none
 */
static inline void uart_reset(void)
{
	reg_rst0 |= FLD_RST0_UART;
	reg_rst0 &= (~FLD_RST0_UART);
}

/**
 * @brief     This function serves to clear tx down.
 * @param[in] none
 * @return    none
 */
static inline void uart_clr_tx_done(void)
{
	reg_uart_state = BIT(7);
}

/**
 * @brief      	This function is used to enable the rtx function of .
 * @return     	none.
 */
static inline void uart_rtx_en()
{
	reg_uart_rx_timeout1 |=FLD_UART_P7816_EN;
}

/**
 * @brief      	This function initializes the UART module.
 * @param[in]  	g_uart_div  -  uart clock divider
 * @param[in]  	g_bwpc      -  bitwidth, should be set to larger than
 *
 *  			sys_clk      baud rate   g_uart_div         g_bwpc
 *
 *	  	  	  	16Mhz        9600          118   			 13
 *           	 	 	 	 19200         118     			 6
 *          	 	 	 	 115200         9       		 13
 *
 *	  	  	  	24Mhz        9600          249       		 9
 *           	 	 	 	 19200		   124               9
 *           	 	 	 	 115200         12    			 15
 *
 *	  	  	  	32Mhz        9600          235       		 13
 *           	 	 	 	 19200		   235               6
 *           	 	 	 	 115200         17    			 13
 *
 *	  	  	  	48Mhz        9600          499       		 9
 *           	 	 	 	 19200		   249               9
 *           	 	 	 	 115200         25    			 15
 *
 * @param[in]  Parity      - selected parity type for UART interface
 * @param[in]  StopBit     - selected length of stop bit for UART interface
 * @return     none
 */
extern void uart_init(unsigned short g_uart_div, unsigned char g_bwpc, UART_ParityTypeDef Parity, UART_StopBitTypeDef StopBit);
/**
 * @brief      This function initializes the UART module.
 * @param[in]  Baudrate  	- uart baud rate
 * @param[in]  System_clock - clock of system
 * @param[in]  Parity      	- selected parity type for UART interface
 * @param[in]  StopBit     	- selected length of stop bit for UART interface
 * @return     none
 */
extern void uart_init_baudrate(unsigned int Baudrate,unsigned int System_clock , UART_ParityTypeDef Parity, UART_StopBitTypeDef StopBit);

/**
 * @brief     enable uart DMA mode
 * @param[in] none
 * @return    none
 */

extern void uart_dma_enable(unsigned char rx_dma_en, unsigned char tx_dma_en);

/**
 * @brief     config the irq of uart tx and rx
 * @param[in] rx_irq_en - 1:enable rx irq. 0:disable rx irq
 * @param[in] tx_irq_en - 1:enable tx irq. 0:disable tx irq
 * @return    none
 */


extern void uart_irq_enable(unsigned char rx_irq_en, unsigned char tx_irq_en);
//use this index to cycle the four register of uart. this index should be reset to 0,when send data after system wakeup.
extern unsigned char uart_TxIndex;
/**
 * @brief     uart send data function with not DMA method.
 *            variable uart_TxIndex,it must cycle the four registers 0x90 0x91 0x92 0x93 for the design of SOC.
 *            so we need variable to remember the index.
 *			  Before entering power-saving mode,you need to call this function in order to ensure that the data sent is completed
 * @param[in] uartData - the data to be send.
 * @return    none
 */
extern void uart_ndma_send_byte(unsigned char uartData);
/**
 * @brief     This function is used to set the 'uart_TxIndex' to 0.
 *			  After wakeup from power-saving mode, you must call this function before sending the data.
 * @param[in] none.
 * @return    none.
 */
static inline void uart_ndma_clear_tx_index(void)
{
    uart_TxIndex=0;
}
/**
 * @brief     config the number level setting the irq bit of status register 0x9d
 *            ie 0x9d[3].
 *          uart_ndma_get_irq  If the cnt register value(0x9c[0,3]) larger or equal than the value of 0x99[0,3]
 *            or the cnt register value(0x9c[4,7]) less or equal than the value of 0x99[4,7],
 *            it will set the irq bit of status register 0x9d, ie 0x9d[3]
 * @param[in] rx_level - receive level value. ie 0x99[0,3]
 * @param[in] tx_level - transmit level value.ie 0x99[4,7]
 * @return    none
 */
extern void uart_ndma_irq_triglevel(unsigned char rx_level, unsigned char tx_level);

/**
 * @brief     get the status of uart irq.
 * @param[in] none
 * @return    0: not uart irq ;
 *            not 0: indicate tx or rx irq
 */

extern unsigned char uart_ndmairq_get(void);

/**
 * @brief     uart send data function, this  function tell the DMA to get data from the RAM and start the DMA transmission
 * @param[in] Addr - pointer to the buffer containing data need to send
 * @return    none
 * @note      If you want to use uart DMA mode to send data, it is recommended to use this function.
 *            This function just triggers the sending action, you can use interrupt or polling with the FLD_UART_TX_DONE flag to judge whether the sending is complete.
 *            After the current packet has been sent, this FLD_UART_TX_DONE will be set to 1, and FLD_UART_TX_DONE interrupt can be generated.
 *			  If you use interrupt mode, you need to call uart_clr_tx_done() in the interrupt processing function, uart_clr_tx_done() will set FLD_UART_TX_DONE to 0.
 *            DMA can only send 2047-bytes one time at most.
 */
extern void uart_send_dma(unsigned char* Addr);

/**
 * @brief     uart send data function, this  function tell the DMA to get data from the RAM and start
 *            the DMA transmission
 * @param[in] Addr - pointer to the buffer containing data need to send
 * @return    1: send success ;
 *            0: DMA busy
 */
extern volatile unsigned char uart_dma_send(unsigned char* Addr);

/**
 * @brief     uart send data function, this  function tell the DMA to get data from the RAM and start
 *            the DMA transmission
 * @param[in] byte - single byte data need to send
 * @return    1: send success ;
 *            0: DMA busy
 */
extern volatile unsigned char uart_send_byte(unsigned char byte);
/**
 * @brief     data receive buffer initiate function. DMA would move received uart data to the address space,
 *            uart packet length needs to be no larger than (recBuffLen - 4).
 * @param[in] RecvAddr - pointer to the receiving buffer
 * @param[in] RecvBufLen - length in byte of the receiving buffer
 * @return    none
 */

extern void uart_recbuff_init(unsigned char *RecvAddr, unsigned short RecvBufLen);



/**
 * @brief     This function determines whether parity error occurs once a packet arrives.
 * @param[in] none
 * @return    1: parity error ;
 *            0: no parity error
 */
extern unsigned char uart_is_parity_error(void);

/**
 * @brief     This function clears parity error status once when it occurs.
 * @param[in] none
 * @return    none
 *
 * Note:
 *(1)DMA mode
 * RX FIFO will also be cleared when parity error flag is cleared .
 *(2)NON-DMA mode
 * When parity error occurs, clear parity error flag after UART receives the data.
 * Cycle the four registers (0x90 0x91 0x92 0x93) from register "0x90" to get data when UART receives the data next time.
 */
extern void  uart_clear_parity_error(void);

/**
 * @brief     UART hardware flow control configuration. Configure RTS pin.
 * @param[in] Enable - enable or disable RTS function.
 * @param[in] Mode - set the mode of RTS(auto or manual).
 * @param[in] Thresh - threshold of trig RTS pin's level toggle(only for auto mode),
 *                     it means the number of bytes that has arrived in Rx buf.
 * @param[in] Invert - whether invert the output of RTS pin(only for auto mode)
 * @param[in] pin   - RTS pin select,it can be GPIO_PA4/GPIO_PB3/GPIO_PB6/GPIO_PC0.
 * @return    none
 */

extern void uart_set_rts(unsigned char Enable, UART_RTSModeTypeDef Mode, unsigned char Thresh, unsigned char Invert,  UART_RtsPinDef pin);

/**
 * @brief     This function sets the RTS pin's level manually
 * @param[in] Polarity - set the output of RTS pin(only for manual mode)
 * @return    none
 */


extern void uart_set_rts_level(unsigned char Polarity);

/**
 * @brief      UART hardware flow control configuration. Configure CTS pin.
 * @param[in]  Enable - enable or disable CTS function.
 * @param[in]  Select - when CTS's input equals to select, tx will be stopped
 * @param[in]  pin   - CTS pin select,it can be GPIO_PA3/GPIO_PB2/GPIO_PC4/GPIO_PD1.
 * @return     none
 */
extern void uart_set_cts(unsigned char Enable, unsigned char Select,UART_CtsPinDef pin);

/**
* @brief      This function serves to select pin for UART module.
* @param[in]  tx_pin   - the pin to send data.
* @param[in]  rx_pin   - the pin to receive data.
* @return     none
*/
extern void uart_gpio_set(UART_TxPinDef tx_pin,UART_RxPinDef rx_pin);

/**
 * @brief   This function enables the irq when UART module receives error data.
 * @param[in] none
 * @return    none
 */
extern void uart_mask_error_irq_enable(void);

#endif

