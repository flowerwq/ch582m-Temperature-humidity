/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : 串口1收发演示
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "stdio.h"
#include "CH58x_common.h"
#include "worktime.h"
#include "configtool.h"
#include "oled.h"
#include "bmp.h"
#include "display.h"
#include "version.h"
#include "modbus.h"
#include "appinfo.h"
#include "uid.h"
#include "sensor.h"
#include "uart.h"
#include "utils.h"
#define TAG "MAIN"
/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */

static int uuid_dump(){
	uint8_t uuid[32] = {0};
	int i = 0;
	GET_UNIQUE_ID(uuid);
	PRINT("flashid:");
	for (i = 0; i < 8; i++){
		PRINT("%02x", uuid[i]);
	}
	PRINT("\r\n");
	cfg_get_sn((char *)uuid);
	PRINT("sn:%s\r\n", uuid);
	return 0;
}
static int reset_dump(){
	SYS_ResetStaTypeDef rst = SYS_GetLastResetSta();
	PRINT("rst(");
	switch(rst){
		case RST_STATUS_SW:
			PRINT("sw reset");
			break;
		case RST_STATUS_RPOR:
			PRINT("poweron");
			break;
		case RST_STATUS_WTR:
			PRINT("wdt");
			break;
		case RST_STATUS_MR:
			PRINT("manual reset");
			break;
		case RST_STATUS_LRM0:
			PRINT("software wakeup");
			break;
		case RST_STATUS_GPWSM:
			PRINT("shutdown wakeup");
			break;
		case RST_STATUS_LRM1:
			PRINT("wdt wakeup");
			break;
		case RST_STATUS_LRM2:
			PRINT("manual wakeup");
			break;
	}
	PRINT(")\r\n");
	return 0;
}

static int debug_init(){
	uart_config_t uart_cfg = UART_DEFAULT_CONFIG();
#if DEBUG == Debug_UART1
	uart_init(UART_NUM_1, &uart_cfg);
#endif
	PRINT("\r\n");
	return 0;
}

int main()
{
	worktime_t worktime = 0;
	char buf[DISPLAY_LINE_LEN + 1];
	WWDG_ResetCfg(ENABLE);
	WWDG_SetCounter(0);
    SetSysClock(CLK_SOURCE_PLL_60MHz);
	worktime_init();
    debug_init();

	reset_dump();
	PRINT("app start ...\r\n");

	WWDG_SetCounter(0);
	OLED_Init();
//	OLED_ShowPicture(32, 0, 64, 64, (uint8_t *)smail_64x64_1, 1);
	oled_seneor_flag();
	OLED_Refresh();

	WWDG_SetCounter(0);
	display_init();
	cfg_init();
	uuid_dump();
	sensor_init();
	
	while(worktime_since(worktime) < 1000){
		WWDG_SetCounter(0);
		__nop();
	}
	//OLED_Clear();
	PRINT("main loop start ...\r\n");
    while(1){
		WWDG_SetCounter(0);
		OLED_Refresh();
//		if (worktime_since(worktime) >= 1000){
//			worktime = worktime_get();
//			if ((worktime / 1000) % 2){
//				OLED_ShowPicture(32, 0, 64, 64, (uint8_t *)smail_64x64_1, 1);
//			}else{
//				OLED_ShowPicture(32, 0, 64, 64, (uint8_t *)smail_64x64_2, 1);
//			}
//		}
		App_sensor_run();
		OLED_Refresh();
		int ret = uart_read(UART_NUM_1, (uint8_t *)buf, sizeof(buf), 100);
		if (ret > 0){
			LOG_DEBUG(TAG, "%d bytes read form uart1", ret);
			log_buffer_hex(TAG, buf,ret, LOG_LEVEL_DEBUG);
		}
	}
}

