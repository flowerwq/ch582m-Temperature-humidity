
/****************************************************************************************/
/*
 *
 * Copyright (C) 2020. Mysentech Inc, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Minyuan Sensing Technology Inc.(Mysentech)  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 *  Please contact <sales@mysentech.com> or contributors for further questions.
*/
/****************************************************************************************/
/* Includes(用户内部MCU配置头文件) ------------------------------------------------------------------*/


/* Includes(MDC04驱动头文件) ------------------------------------------------------------------*/
#include "MDC04_driver.h"
#include "bsp_SysTick.h"
#include "MY_ow.h"

extern float CapCfg_offset, CapCfg_range;

/*
  * @brief  读取Rom id函数
*/
int MY_Read_ROM(void)
{ 
	uint8_t rom_id[8];
	
  MDC04_ReadROM(rom_id);
	printf("\r\n MDC04 ROMID :");				
	for(int i=0;i<8;i++)
	{
	printf("%2x ", rom_id[i]);				
	}		
	return 1;
}
/*
  * @brief  测量温度函数
*/
int MY_T(void)
{ 
	float fTemp; uint16_t iTemp; 
	
	if(ConvertTemp() == TRUE)
	{
    ow_Delay_ms(15);
		ReadTempWaiting(&iTemp);
		fTemp=MDC04_OutputtoTemp((int16_t)iTemp); 
		printf("\n\r T= %3.3f ", fTemp);
	}
	else
	{
		printf("\r\n No MDC04");
	}	
	
  ow_Delay_ms(990);
				
	return 1;
}
/*
  * @brief  测量温度+通道1电容函数
*/
int MY_TC1(void)
{	
	uint16_t iTemp, iCap1; 
	float fTemp, fCap1;
		
	 ReadCapConfigure(&CapCfg_offset, &CapCfg_range);
	 SetCapChannel(CAP_CH1_SEL);
	
		if(ConvertTC1() == TRUE)
		{
			ow_Delay_ms(15);		
			if(ReadTempCap1(&iTemp, &iCap1) == TRUE) 
			{
				fTemp=MDC04_OutputtoTemp(iTemp);
				fCap1=MDC04_OutputtoCap(iCap1, CapCfg_offset, CapCfg_range);
				printf("\r\n T= %3.3f C C1= %6.3f pF", fTemp, fCap1);	
			}
		}
		else
		{
			printf("\r\n No MDC04");
		}
		ow_Delay_ms(990);
	return 1;		
}

/*
  * @brief  测量四通道电容函数
*/
int MY_C(void)
{	
	float fcap1, fcap2, fcap3, fcap4; uint16_t iTemp, icap1, icap[3];
	uint8_t status, cfg;
		
	SetCapChannel(CAP_CH1CH2CH3CH4_SEL);
	ReadStatusConfig((uint8_t *)&status, (uint8_t *)&cfg);
	
	if(ConvertCap() == FALSE)
	{
		printf("\r\nNo MDC04");
	}else
	{
		
		ow_Delay_ms(15);
		ReadCapConfigure(&CapCfg_offset, &CapCfg_range);
		{
		ReadStatusConfig((uint8_t *)&status, (uint8_t *)&cfg);
		ReadTempCap1(&iTemp, &icap1);
		ReadCapC2C3C4(icap);

		fcap1 = MDC04_OutputtoCap(icap1, CapCfg_offset, CapCfg_range);
		fcap2 = MDC04_OutputtoCap(icap[0], CapCfg_offset, CapCfg_range);
		fcap3 = MDC04_OutputtoCap(icap[1], CapCfg_offset, CapCfg_range);
		fcap4=  MDC04_OutputtoCap(icap[2], CapCfg_offset, CapCfg_range);
		printf("\r\n C1=%5d , %6.3f  C2=%5d, %6.3f  C3=%5d, %6.3f  C4=%5d, %6.3f  SC=%02X%02X", icap1, fcap1, icap[0], fcap2, icap[1], fcap3, icap[2], fcap4, status, cfg);
		}
	}
	ow_Delay_ms(990);
	return 1;		
}

/*
  * @brief  Polling模式下读取温度函数
*/
int MY_P(void)
{ 
	uint16_t iTemp;
	
	ConvertTemp();
	ReadTempPolling(&iTemp);
	MDC04_OutputtoTemp((int16_t)iTemp); 
	
	return 1;
}

/*
  * @brief  设置配置寄存器
  * MPS:  000   001     010    011    100   101
  *      单次  0.5次/S 1次/S  2次/S  4次/S 10次/S
  * Repeatability:  00: 低重复性
  *                 01：中重复性
  *                 10：高重复性
*/
int MY_F(int repeatability,int mps)
{ 
	int status, cfg;
	SetConfig(mps & 0x07, repeatability & 0x03);
	ReadStatusConfig((uint8_t *)&status, (uint8_t *)&cfg);
	printf("S=%02x C=%02x", status, cfg);
	
	return 0;
}

/*
  * @brief      设置电容测量通道
  * Cap1					0x01 		
  * Cap2					0x02
  * Cap3					0x03
  * Cap4					0x04
  * Cap1_2				0x05
  * Cap1_2_3			0x06
  * Cap1_2_3_4		0x07
*/
int MY_Channel(uint8_t channel)
{ 
			
		SetCapChannel(channel);
									
		return 1;
}

/*
  * @brief  设置偏置电容offset
*/
int MY_Offset(float Co)
{ 
				
		printf("\r\nCo= %5.2f", Co);
			
		if(!((Co >=0.0) && (Co <= 103.5))) 
		{
		printf(" %s", "The input is out of range"); 
			return 0;
		}
		
		MDC04_CapConfigureOffset(Co);
		
		return 1;
}

/*
  * @brief  设置量程电容
  * 默认出厂配置量程电容±15.5pf
*/
int MY_FullScale(float Cr)
{  	
	   printf("\r\nCr= %5.3f", Cr);
			
	   if(!((Cr >=0.0) && (Cr <= 15.5))) 
		{printf(" %s", "The input is out of range"); return 0;}
		
		MDC04_CapConfigureFs(Cr);
	
		ReadCapConfigure(&CapCfg_offset, &CapCfg_range);	
		
		return 1;
}
/*
  * @brief  设置电容测量范围
  * 请勿设置超出电容量程0~119pf,请勿超出最大range:±15.5pf
*/
int MY_Range(float Cmin,float Cmax)
{ 
//		printf("\r\nCmin= %3.2f Cmax=%3.2f", Cmin, Cmax);
			
		if(!((Cmax <= 119.0) && (Cmax > Cmin) && (Cmin >= 0.0) && ((Cmax-Cmin) <= 31.0)))  
		{printf(" %s", "The input is out of range"); return 0;}
		
		MDC04_CapConfigureRange(Cmin, Cmax);
		
		ReadCapConfigure(&CapCfg_offset, &CapCfg_range);
		
		return 1;
}

/*
  * @brief 将暂存器内配置存入EEPROM
*/
int MY_EEPROM(void)
{
	
	SavetoE2PROMPage0();
	
	return 1;
}
