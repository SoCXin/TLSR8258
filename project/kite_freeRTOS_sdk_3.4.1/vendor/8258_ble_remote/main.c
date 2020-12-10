/********************************************************************************************************
 * @file	 main.c
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 18, 2018
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

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "vendor/common/blt_fw_sign.h"
#include "../../vendor/common/user_config.h"
#include "freertos_api.h"
#include "../../common/breakpoint.h"

#define MY_TIMER_TASK_TEST_ENABLE				0

extern void user_init_normal();
extern void user_init_deepRetn();
extern void main_loop (void);
extern void deep_wakeup_proc(void);
extern void rc_ir_irq_prc(void);
extern void xPortTimer0Handler(void);
extern void user_init();

_attribute_ram_code_ void irq_handler(void)
{
#if (REMOTE_IR_ENABLE)
	rc_ir_irq_prc();
#endif


	#if(OS_ENABLE)
	u32 src = reg_irq_src;
	int timerSwitching = (src & FLD_IRQ_TMR0_EN);
	int allow = irq_allow_task_switch();
	if(timerSwitching){
		DEBUG_GPIO(GPIO_CHN7, 1);

		reg_tmr_sta = FLD_TMR_STA_TMR0; 		//clear irq status
		reg_tmr0_tick = 0;
		if(allow && xTaskIncrementTick() != pdFALSE){
			void *old = (void*)xTaskGetCurrentTaskHandle();
			vTaskSwitchContext();
			void *nw = (void*)xTaskGetCurrentTaskHandle();

			if(old != nw){
				portDISABLE_INTERRUPTS();
				vPortYieldSvc(old, nw);
			}
			return;
		}

	}
	#endif
	irq_blt_sdk_handler ();

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)
	extern void irq_phyTest_handler(void);
	irq_phyTest_handler();
#endif
	DEBUG_GPIO(GPIO_CHN7, 0);

}

/* 优先级数越大，优先级越高，与μcos相反*/

#define TASK_PROT_PRIORITY	( tskIDLE_PRIORITY + 3 )
#define TASK_UI_PRIORITY	( tskIDLE_PRIORITY + 1 )
#define TASK_ANCS_PRIORITY	( tskIDLE_PRIORITY + 2 )




extern TaskHandle_t handle_proto_task;
extern int proto_task( void *pvParameters );
extern void task_restore(int);

TaskHandle_t handle_ui;
static void ui_task( void *pvParameters ){
	while(1){
		main_loop();
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
		printf("Entry main loop()!!!\n");
	}
}

TaskHandle_t handle_task0;

struct _System_Test_TaskQ
{
    u32  event;
   void* data;
};

typedef struct _System_Test_TaskQ System_Test_TaskQ_t;

_attribute_data_retention_ TaskHandle_t g_test_taskHandle;
_attribute_data_retention_ static TimerHandle_t g_test_xTimer = NULL;
_attribute_data_retention_ void *g_test_taskQ = NULL;

void System_TestTimer_Callback( TimerHandle_t xTimer )
{
    if (g_test_taskQ != NULL)
    {
        System_Test_TaskQ_t ewQ;
        ewQ.event = 1;
       ewQ.data = &g_test_taskHandle;
      //  printf("system test timer callback\r\n");
       xQueueSend((xQueueHandle)g_test_taskQ, &ewQ, 0);
    }
}
void System_Test_Task_Setup (void)
{
    g_test_xTimer = xTimerCreate("Test",
               /**定时溢出周期， 单位是任务节拍数*/
    			22/portTICK_PERIOD_MS,
               /**是否自动重载*/
               pdFALSE,
              ( void * ) 0,
               /*回调函数*/
              System_TestTimer_Callback);

    g_test_taskQ = xQueueCreate(8,sizeof(System_Test_TaskQ_t));  /** 创建队列*/

		//printf("system test task setup \r\n");

}

void System_Test_Task_Run (void *pvParameters)
{
    static System_Test_TaskQ_t ewQ;
    ewQ.event =0;

    while (1)
    {
        switch(ewQ.event)
        {
            case 0:
                {
                    xTimerChangePeriod( g_test_xTimer, 1000/portTICK_PERIOD_MS, portMAX_DELAY);
                    xTimerStart( g_test_xTimer, 0 );

                }
                break;
            case 1:
                {
					sleep_us(10*1000);
                    xTimerStop(g_test_xTimer, 0 );
//					u32 interval = get_timer_task_interval(10);
                    xTimerChangePeriod( g_test_xTimer, 10 / portTICK_PERIOD_MS, portMAX_DELAY);
                    xTimerStart( g_test_xTimer, 0 );

                }
                break;
            default:
                break;
        }
        xQueueReceive((xQueueHandle)g_test_taskQ, &ewQ, portMAX_DELAY);
        printf("system test task\r\n");

    }

}

#if (BATT_CHECK_ENABLE)
extern u8 adc_hw_initialized;
#endif

//void myDbgInit(u32 pin){
//	gpio_set_func(pin, AS_GPIO);
//	gpio_set_output_en(pin, 1);
//	gpio_set_input_en(pin, 0);
//	gpio_write(pin, 0);
//}

_attribute_ram_code_ int main(void)    //must run in ramcode
{

	blc_pm_select_internal_32k_crystal();
//	blc_pm_select_external_32k_crystal();
	cpu_wakeup_init();
#if (BATT_CHECK_ENABLE)
	adc_hw_initialized = 0;
#endif
	blt_dma_tx_rptr = 0;

	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	rf_drv_init(RF_MODE_BLE_1M);

	gpio_init( !deepRetWakeUp );  //analog resistance will keep available in deepSleep mode, so no need initialize again

#if (CLOCK_SYS_CLOCK_HZ == 16000000)
	clock_init(SYS_CLK_16M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
	clock_init(SYS_CLK_24M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
	clock_init(SYS_CLK_32M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
	clock_init(SYS_CLK_48M_Crystal);
#endif

	//deep_wakeup_proc();

	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value

	if( deepRetWakeUp ){
		user_init_deepRetn ();
	}
	else{
		#if FIRMWARES_SIGNATURE_ENABLE
			blt_firmware_signature_check();
		#endif
		user_init_normal ();
	}

	#if(OS_ENABLE)

	if(deepRetWakeUp){
//		gpio_write(GPIO_PB1, 0);
		task_restore(1);
		// never reach here
	}else{
		System_Test_Task_Setup();
		xTaskCreate( proto_task, "tProto", 128, (void*)0, TASK_PROT_PRIORITY, &handle_proto_task );
		xTaskCreate( ui_task, "tUI", configMINIMAL_STACK_SIZE, (void*)0, TASK_ANCS_PRIORITY, &handle_ui );
		xTaskCreate( System_Test_Task_Run, "tTASK0", configMINIMAL_STACK_SIZE, (void*)0, TASK_UI_PRIORITY, &handle_task0 );
		vTaskStartScheduler();
		// never reach here
	}

	#else
    irq_enable();
	while(1){
		blt_sdk_main_loop();
	}
	#endif


}




