/********************************************************************************************************
 * @file	driver_8258.h
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



#include "drivers/8258/bsp.h"
#include "drivers/8258/aes.h"
#include "drivers/8258/analog.h"
#include "drivers/8258/compiler.h"
#include "drivers/8258/register.h"
#include "drivers/8258/gpio.h"
#include "drivers/8258/pwm.h"
#include "drivers/8258/irq.h"
#include "drivers/8258/clock.h"
#include "drivers/8258/random.h"
#include "drivers/8258/flash.h"
#include "drivers/8258/rf_drv.h"
#include "drivers/8258/pm.h"
#include "drivers/8258/audio.h"
#include "drivers/8258/adc.h"
#include "drivers/8258/i2c.h"
#include "drivers/8258/spi.h"
#include "drivers/8258/uart.h"
#include "drivers/8258/register.h"
#include "drivers/8258/watchdog.h"
#include "drivers/8258/register.h"
#include "drivers/8258/dfifo.h"
#include "drivers/8258/dma.h"
#include "drivers/8258/emi.h"
#include "drivers/8258/timer.h"

#include "drivers/8258/s7816.h"
#include "drivers/8258/qdec.h"
#include "drivers/8258/lpc.h"

#include "drivers/8258/rf_pa.h"
