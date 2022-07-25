#ifndef __OLED_H
#define __OLED_H 

#include "stdlib.h"	
#include "stdint.h"	

#define OLED_X_MAX	128
#define OLED_Y_MAX	64

#define OLED_X_VALID(x)	((x) >= 0 && (x) < OLED_X_MAX)
#define OLED_Y_VALID(y)	((y) >= 0 && (y) < OLED_Y_MAX)

//-----------------OLED端口定义----------------
#define OLED_SCL_INIT() {GPIOA_ModeCfg(GPIO_Pin_0, GPIO_ModeOut_PP_20mA);}
#define OLED_SCL_Clr() GPIOA_ResetBits(GPIO_Pin_0)//SCL
#define OLED_SCL_Set() GPIOA_SetBits(GPIO_Pin_0)

#define OLED_SDA_INIT() {\
        GPIOA_ModeCfg(GPIO_Pin_1, GPIO_ModeOut_PP_20mA);\
        GPIOA_SetBits(GPIO_Pin_1);\
    }
#define OLED_SDA_Clr() GPIOA_ResetBits(GPIO_Pin_1)//SDAA
#define OLED_SDA_Set() GPIOA_SetBits(GPIO_Pin_1)

#define OLED_RES_INIT() {\
        GPIOB_ModeCfg(GPIO_Pin_3, GPIO_ModeOut_PP_20mA);\
        GPIOB_SetBits(GPIO_Pin_3);\
    }
#define OLED_RES_Clr() GPIOB_ResetBits(GPIO_Pin_3)//RES
#define OLED_RES_Set() GPIOB_SetBits(GPIO_Pin_3)

#define OLED_DC_INIT()  {\
        GPIOB_ModeCfg(GPIO_Pin_2, GPIO_ModeOut_PP_20mA);\
        GPIOB_SetBits(GPIO_Pin_2);\
    }
#define OLED_DC_Clr()  GPIOB_ResetBits(GPIO_Pin_2)//DC
#define OLED_DC_Set()  GPIOB_SetBits(GPIO_Pin_2)

#define OLED_CT_INIT()  {\
        GPIOB_ModeCfg(GPIO_Pin_5, GPIO_ModeOut_PP_5mA);\
    }
#define OLED_CT()  GPIOB_ResetBits(GPIO_Pin_5);

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

typedef struct oled_area {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
}oled_area_t;

void OLED_ClearPoint(uint8_t x,uint8_t y);
void OLED_ColorTurn(uint8_t i);
void OLED_DisplayTurn(uint8_t i);
void OLED_WR_Byte(uint8_t dat,uint8_t mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t mode);
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode);
void OLED_ShowChar6x8(uint8_t x,uint8_t y,uint8_t chr,uint8_t mode);
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t mode);
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t mode);
void OLED_ScrollDisplay(uint8_t num,uint8_t space,uint8_t mode);
void OLED_ShowPicture(uint8_t x,uint8_t y,uint8_t sizex,uint8_t sizey,uint8_t BMP[],uint8_t mode);
void OLED_Init(void);
void OLED_clear_buffer(oled_area_t *area);

void oled_seneor_flag();
#endif

