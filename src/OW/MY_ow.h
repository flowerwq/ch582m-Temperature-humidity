/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MY_OW_H
#define MY_OW_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "MY_stdtype.h"

//***********需要根据用户的延时函数进行相应替换，务必是准确的us级延时！*********//
#define ow_Delay_us			DelayUs
#define ow_Delay_ms(x) DelayMs(x)
/* Definition of DQ pin for one-wire communication*/

#define GPIOOW_DQ_GPIO_PORT_B
//#define GPIOOW_DQ_GPIO_PORT_A

#define GPIOOW_DQ_PIN                   GPIO_Pin_8

#ifdef GPIOOW_DQ_GPIO_PORT_B

/* Macros for DQ manipulation*/
#define ow_DQ_init()	{GPIOB_ModeCfg(GPIOOW_DQ_PIN, GPIO_ModeOut_PP_5mA);}
#define ow_DQ_set()   		{ GPIOB_SetBits(GPIOOW_DQ_PIN);}
#define ow_DQ_reset() 		{ GPIOB_ResetBits(GPIOOW_DQ_PIN); }
#define ow_DQ_get()   		( GPIOB_ReadPortPin(GPIOOW_DQ_PIN ))
#elif defined(GPIOOW_DQ_GPIO_PORT_A)
#define ow_DQ_init()	{GPIOA_ModeCfg(GPIOOW_DQ_PIN, GPIO_ModeOut_PP_5mA);}
#define ow_DQ_set()   		{ GPIOA_SetBits(GPIOOW_DQ_PIN);}
#define ow_DQ_reset() 		{ GPIOA_ResetBits(GPIOOW_DQ_PIN); }
#define ow_DQ_get()   		( GPIOA_ReadPortPin(GPIOOW_DQ_PIN ))

#else
#error "invalid dq io port"
#endif

typedef enum {
  READY       = 0,
  BUSY    		= 1
} OW_SLAVESTATUS;

/* Exported_Functions----------------------------------------------------------*/
OW_SLAVESTATUS OW_ReadStatus(void);
void OW_Init(void);
bool OW_ResetPresence(void);
void OW_WriteByte(uint8_t data);
uint8_t OW_ReadByte(void);
uint8_t OW_Read2Bits(void);
uint8_t CRC8_Cal(uint8_t *serial, uint8_t length);


#endif /* MY_OW_H */
