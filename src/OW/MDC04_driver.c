
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

/* Includes ------------------------------------------------------------------*/
//#include "system.h"
#include <math.h>

/* Includes(MDC04驱动头文件) ------------------------------------------------------------------*/
#include "MDC04_driver.h"
#include "bsp_SysTick.h"
#include "MY_ow.h"

/****全局变量：保存和电容配置寄存器对应的偏置电容和量程电容数值****/
float CapCfg_offset, CapCfg_range;
uint8_t CapCfg_ChanMap, CapCfg_Chan;

/****偏置电容和反馈电容阵列权系数****/
static const float COS_Factor[8] = {0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 40.0};
/*Cos= (40.0*q[7]+32.0*q[6]+16.0*q[5]+8.0*q[4]+4.0*q[3]+2.0*q[2]+1.0*q[1]+0.5*q[0])*/
static const struct  {float Cfb0; float Factor[6];} CFB = { 2.0, 2.0, 4.0, 8.0, 16.0, 32.0, 46.0};
/*Cfb =(46*p[5]+32*p[4]+16*p[3]+8*p[2]+4*p[1]+2*p[0]+2)*/

/**
  * @brief  把16位二进制补码表示的温度输出转换为以摄氏度为单位的温度读数
  * @param  out：有符号的16位二进制温度输出
  * @retval 以摄氏度为单位的浮点温度
*/
float MDC04_OutputtoTemp(int16_t out)
{
	return ((float)out/256.0 + 40.0);
}

/**
  * @brief  把以摄氏度为单位的浮点温度值转换为16位二进制补码表示的温度值
  * @param  以摄氏度为单位的浮点温度值
  * @retval 有符号的16位二进制温度值
*/
int16_t MDC04_TemptoOutput(float Temp)
{
	return (int16_t)((Temp-40.0)*256.0);
}

/**
  * @brief  把16位二进制电容输出转换为以pF为单位的电容读数
  * @param  out：无符号的16位二进制电容输出
  * @param  Co：配置的偏置电容数值
  * @param  Cr：配置的范围电容（量程）数值
  * @retval 以pF为单位的浮点电容数值
*/
float MDC04_OutputtoCap(uint16_t out, float Co, float Cr)
{
	return (2.0*(out/65535.0-0.5)*Cr+Co);
}

/**
  * @brief  计算多个字节序列的校验和
  * @param  serial：字节数组指针
  * @param  length：字节数组的长度
  * @retval 校验和（CRC）
*/
#define POLYNOMIAL 	0x131 //100110001
uint8_t MY_OW_CRC8(uint8_t *serial, uint8_t length)
{
    uint8_t result = 0x00;
    uint8_t pDataBuf;
    uint8_t i;

    while(length--) {
        pDataBuf = *serial++;
        for(i=0; i<8; i++) {
            if((result^(pDataBuf))&0x01) {
                result ^= 0x18;
                result >>= 1;
                result |= 0x80;
            }
            else {
                result >>= 1;
            }
            pDataBuf >>= 1;
        }
    }

    return result;
}

bool MDC04_nReadScratchpad_SkipRom(uint8_t *scr, uint8_t size)
{
    int16_t i;

	/*size < sizeof(MDC04_SCRATCHPAD_READ)*/
    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(READ_SCRATCHPAD);

		for(i=0; i<size; i++)
    {
			*scr++ = OW_ReadByte();
		}

    return TRUE;
}
/**
  * @brief  读芯片ROM ID
*/
bool MDC04_ReadROM(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;
		
    OW_WriteByte(READ_ROM);

		for(i=0; i < sizeof(MDC04_ROMCODE); i++)
    {
			*scr++ = OW_ReadByte();
		}

    return TRUE;
}

/**
  * @brief  读芯片寄存器的暂存器组
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_SCRATCHPAD_READ）
  * @retval 读状态
*/
bool MDC04_ReadScratchpad_SkipRom(uint8_t *scr)
{
    int16_t i;

	/*size < sizeof(MDC04_SCRATCHPAD_READ)*/
    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(READ_SCRATCHPAD);

		for(i=0; i < sizeof(MDC04_SCRATCHPAD_READ); i++)
    {
			*scr++ = OW_ReadByte();
		}

    return TRUE;
}
/**
  * @brief  写芯片寄存器的暂存器组
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_SCRATCHPAD_WRITE）
  * @retval 写状态
**/
bool MDC04_WriteScratchpad_SkipRom(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(WRITE_SCRATCHPAD);

		for(i=0; i < sizeof(MDC04_SCRATCHPAD_WRITE); i++)
    {
			OW_WriteByte(*scr++);
		}

    return TRUE;
}
/**
  * @brief  读芯片寄存器的扩展暂存器组
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_SCRATCHPADEXT）
  * @retval 读状态
**/
bool MDC04_ReadScratchpadExt_SkipRom(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(READ_SCRATCHPAD_EXT);

		for(i=0; i< sizeof(MDC04_SCRATCHPADEXT); i++)
    {
			*scr++ = OW_ReadByte();
		}

    return TRUE;
}
/**
  * @brief  写芯片寄存器的扩展暂存器组
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_SCRATCHPADEXT）
  * @retval 写状态
**/
bool MDC04_WriteScratchpadExt_SkipRom(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(WRITE_SCRATCHPAD_EXT);

		for(i=0; i<sizeof(MDC04_SCRATCHPADEXT)-1; i++)
    {
			OW_WriteByte(*scr++);
		}

    return TRUE;
}
/**
  * @brief  读电容通道2，3，4测量结果寄存器的内容
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_C2C3C4）
  * @retval 写状态
**/
bool MDC04_ReadC2C3C4_SkipRom(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(READ_C2C3C4);

		for(i=0; i < sizeof(MDC04_C2C3C4); i++)
    {
			*scr++ = OW_ReadByte();
		}

    return TRUE;
}
/**
  * @brief  读芯片寄存器的参数组
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_SCRPARAMETERS）
  * @retval 读状态
**/
bool MDC04_ReadParameters_SkipRom(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(READ_PARAMETERS);

		for(i=0; i < sizeof(MDC04_SCRPARAMETERS); i++)
    {
			*scr++ = OW_ReadByte();
		}

    return TRUE;
}
/**
  * @brief  写芯片寄存器的参数组
  * @param  scr：字节数组指针， 长度为 @sizeof（MDC04_SCRPARAMETERS）
  * @retval 写状态
**/
bool MDC04_WriteParameters_SkipRom(uint8_t *scr)
{
    int16_t i;

    if(OW_ResetPresence() == FALSE)
			return FALSE;

    OW_WriteByte(SKIP_ROM);
    OW_WriteByte(WRITE_PARAMETERS);

		for(i=0; i < sizeof(MDC04_SCRPARAMETERS); i++)
    {
			OW_WriteByte(*scr++);
		}

    return TRUE;
}

/**
  * @brief  保存暂存器和扩展暂存器的内容到EEPROM的Page0，并等待编程结束
  * @param  无
  * @retval 状态
**/
bool SavetoE2PROMPage0(void)
{
	if(OW_ResetPresence() == FALSE)
		return FALSE;

	OW_WriteByte(SKIP_ROM);
	OW_WriteByte(COPY_PAGE0);

	/*等待擦除和编程完成*/
	ow_Delay_ms(45);

  return TRUE;
}

/**
  * @brief  启动温度测量
  * @param  无
  * @retval 单总线发送状态
*/
bool ConvertTemp(void)
{
	if(OW_ResetPresence() == FALSE)
		return FALSE;

  OW_WriteByte(SKIP_ROM);
  OW_WriteByte(CONVERT_T);

  return TRUE;
}

/**
  * @brief  启动温度和电容通道1同时测量
  * @param  无
  * @retval 单总线发送状态
*/
bool ConvertTC1(void)
{
	if(OW_ResetPresence() == FALSE)
		return FALSE;

  OW_WriteByte(SKIP_ROM);
  OW_WriteByte(CONVERT_TC1);

  return TRUE;
}

/**
  * @brief  启动（多个通道）电容测量
  * @param  无
  * @retval 单总线发送状态
*/
bool ConvertCap(void)
{
	if(OW_ResetPresence() == FALSE)
		return FALSE;

  OW_WriteByte(SKIP_ROM);
  OW_WriteByte(CONVERT_C);

  return TRUE;
}

/**
  * @brief  等待转换结束后读测量结果。和@ConvertTemp联合使用
  * @param  iTemp：返回的16位温度测量结果
  * @retval 读状态
*/
bool ReadTempWaiting(uint16_t *iTemp)
{
	uint8_t scrb[sizeof(MDC04_SCRATCHPAD_READ)];
	MDC04_SCRATCHPAD_READ *scr = (MDC04_SCRATCHPAD_READ *) scrb;

	/*读9个字节。前两个是温度转换结果，最后字节是前8个的校验和--CRC。*/
	if(MDC04_ReadScratchpad_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
  if(scrb[8] != MY_OW_CRC8(scrb, 8))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	/*将温度测量结果的两个字节合成为16位字。*/
	*iTemp=(uint16_t)scr->T_msb<<8 | scr->T_lsb;

  return TRUE;
}

/**
  * @brief  查询是否转换结束，然后读测量结果。和@ConvertTemp联合使用
  * @param  iTemp：返回的16温度测量结果
  * @retval 读结果状态
*/
bool ReadTempPolling(uint16_t *iTemp)
{ int timeout = 0;

	/*读状态位时隙。如果转换还没结束，芯片以1响应读时隙。如果转换结束，芯片以0响应度时隙。
	前两个字节是温度转换结果，最后字节是前8个的校验和--CRC。*/
	while (OW_ReadStatus() == BUSY )
	{
		ow_Delay_ms(1);
    timeout++;
		if(timeout > 50)
		{
			return FALSE;				/*超时错误*/
		}
	}

	uint8_t scrb[sizeof(MDC04_SCRATCHPAD_READ)];
	MDC04_SCRATCHPAD_READ *scr = (MDC04_SCRATCHPAD_READ *) scrb;

	/*读9个字节。前两个是温度转换结果，最后字节是前8个的校验和--CRC。*/
	if(MDC04_ReadScratchpad_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*I2C地址头应答为NACK*/
	}

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
  if(scrb[8] != MY_OW_CRC8(scrb, 8))
  {
		return FALSE;  /*CRC验证未通过*/
  }
	/*将温度测量结果的两个字节合成为16位字。*/
	*iTemp=(uint16_t)scr->T_msb<<8 | scr->T_lsb;

  return TRUE;
}

/**
  * @brief  等待转换结束后读测量结果。和@ConvertTC1联合使用
  * @param  iTemp：返回的16位温度测量结果
  * @param  iCap1：返回的16位电容通道1测量结果
  * @retval 读结果状态
*/
bool ReadTempCap1(uint16_t *iTemp, uint16_t *iCap1)
{
	uint8_t scrb[sizeof(MDC04_SCRATCHPAD_READ)];
	MDC04_SCRATCHPAD_READ *scr = (MDC04_SCRATCHPAD_READ *) scrb;

	/*读9个字节。前两个是温度转换结果，最后字节是前8个的校验和--CRC。*/
	if(MDC04_ReadScratchpad_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
  if(scrb[8] != MY_OW_CRC8(scrb, 8))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	*iTemp=(uint16_t)scr->T_msb<<8 | scr->T_lsb;
	*iCap1=(uint16_t)scr->C1_msb<<8 | scr->C1_lsb;

  return TRUE;
}

/**
  * @brief  查询是否转换结束，然后读测量结果。和 @ConvertTC1联合使用
  * @param  iTemp：返回的16温度测量结果
  * @param  iCap1：返回的16电容1测量结果
  * @retval 读结果状态
*/
bool ReadTempCap1Polling(uint16_t *iTemp, uint16_t *iCap1)
{ int timeout=0;

	/*读状态位时隙。如果转换还没结束，芯片以1响应读时隙。如果转换结束，芯片以0响应度时隙。
	前两个字节是温度转换结果，最后字节是前8个的校验和--CRC。*/
	while (OW_ReadStatus() == BUSY )
	{
		ow_Delay_ms(1);
    timeout++;
		if(timeout > 50)
		{
			return FALSE;				/*超时错误*/
		}
	}

	uint8_t scrb[sizeof(MDC04_SCRATCHPAD_READ)];
	MDC04_SCRATCHPAD_READ *scr = (MDC04_SCRATCHPAD_READ *) scrb;

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
	if(MDC04_ReadScratchpad_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*I2C地址头应答为NACK*/
	}

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
  if(scrb[8] != MY_OW_CRC8(scrb, 8))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	*iTemp=(uint16_t)scr->T_msb<<8 | scr->T_lsb;
	*iCap1=(uint16_t)scr->C1_msb<<8 | scr->C1_lsb;

  return TRUE;
}

/**
  * @brief  读电容通道2，3和4的测量结果。和 @ConvertCap联合使用
  * @param  icap：数组指针
  * @retval 读结果状态
*/
bool ReadCapC2C3C4(uint16_t *iCap)
{
	uint8_t scrb[sizeof(MDC04_C2C3C4)];
	MDC04_C2C3C4 *scr = (MDC04_C2C3C4 *) scrb;

	/*读6个字节。每两个字节依序分别为通道2、3和4的测量结果，最后字节是前两个的校验和--CRC。*/
	if(MDC04_ReadC2C3C4_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前两个字节的校验和，并与接收的第3个CRC字节比较。*/
//  if(scrb[8] != MY_OW_CRC8(scrb, 8))
//  {
//		return FALSE;  /*CRC验证未通过*/
//  }

	iCap[0] = (uint16_t)scr->C2_msb<<8 | scr->C2_lsb;
	iCap[1] = (uint16_t)scr->C3_msb<<8 | scr->C3_lsb;
	iCap[2] = (uint16_t)scr->C4_msb<<8 | scr->C4_lsb;

  return TRUE;
}

/**
  * @brief  读偏置电容配置寄存器内容
  * @param  Coffset：偏置配置寄存器有效位的内容
  * @retval 无
*/
bool ReadCosConfig(uint8_t *Coscfg)
{
	uint8_t scrb[sizeof(MDC04_SCRPARAMETERS)];
	MDC04_SCRPARAMETERS *scr = (MDC04_SCRPARAMETERS *) scrb;

	/*读15个字节。第5字节是偏置电容配置寄存器，第10字节是量程电容配置寄存器，最后字节是前14个的校验和--CRC。*/
	if(MDC04_ReadParameters_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前14个字节的校验和，并与接收的第15个CRC字节比较。*/
  if(scrb[sizeof(MDC04_SCRPARAMETERS)-1] != MY_OW_CRC8(scrb, sizeof(MDC04_SCRPARAMETERS)-1))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	*Coscfg = scr->Cos & (0xFF >> (3 - (scr->Cfb >> 6))); //屏蔽掉无效位，根据CFB寄存器的高2位

  return TRUE;
}

/**
  * @brief  写偏置电容配置寄存器和有效位宽设置
  * @param  Coffset：偏置配置寄存器的数值
  * @param  Cosbits：偏置配置寄存器有效位宽，可能为：
	*		@COS_RANGE_5BIT
	*		@COS_RANGE_6BIT
	*		@COS_RANGE_7BIT
	*		@COS_RANGE_8BIT
  * @retval 状态
*/
bool WriteCosConfig(uint8_t Coffset, uint8_t Cosbits)
{
	uint8_t scrb[sizeof(MDC04_SCRPARAMETERS)];
	MDC04_SCRPARAMETERS *scr = (MDC04_SCRPARAMETERS *) scrb;

	/*读15个字节。第5字节是偏置电容配置寄存器，第10字节是量程电容配置寄存器，最后字节是前14个的校验和--CRC。*/
	if(MDC04_ReadParameters_SkipRom(scrb) == FALSE)
	{
		return FALSE;   /*读寄存器失败*/
	}

	/*计算接收的前14个字节的校验和，并与接收的第15个CRC字节比较。*/
  if(scrb[sizeof(MDC04_SCRPARAMETERS)-1] != MY_OW_CRC8(scrb, sizeof(MDC04_SCRPARAMETERS)-1))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	scr->Cos = Coffset;
	scr->Cfb = (scr->Cfb & ~CFB_COSRANGE_Mask) | Cosbits;

	MDC04_WriteParameters_SkipRom(scrb);

  return TRUE;
}

/**
  * @brief  读量程电容配置寄存器内容
  * @param  Cfb：量程配置寄存器低6位的内容
  * @retval 状态
*/
bool ReadCfbConfig(uint8_t *Cfb)
{
	uint8_t scrb[sizeof(MDC04_SCRPARAMETERS)];
	MDC04_SCRPARAMETERS *scr = (MDC04_SCRPARAMETERS *) scrb;

	/*读15个字节。第5字节是偏置电容配置寄存器，第10字节是量程电容配置寄存器，最后字节是前14个的校验和--CRC。*/
	if(MDC04_ReadParameters_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前14个字节的校验和，并与接收的第15个CRC字节比较。*/
  if(scrb[sizeof(MDC04_SCRPARAMETERS)-1] != MY_OW_CRC8(scrb, sizeof(MDC04_SCRPARAMETERS)-1))
  {
		return FALSE;   /*CRC验证未通过*/
  }

	*Cfb = scr->Cfb & MDC04_CFEED_CFB_MASK;

  return TRUE;;
}

/**
  * @brief  写量程电容配置寄存器
  * @param  Cfb：量程配置寄存器低6位的内容
  * @retval 状态
*/
bool WriteCfbConfig(uint8_t Cfb)
{
	uint8_t scrb[sizeof(MDC04_SCRPARAMETERS)];
	MDC04_SCRPARAMETERS *scr = (MDC04_SCRPARAMETERS *) scrb;

	/*读15个字节。第5字节是偏置电容配置寄存器，第10字节是量程电容配置寄存器，最后字节是前14个的校验和--CRC。*/
	if(MDC04_ReadParameters_SkipRom(scrb) == FALSE)
	{
		return FALSE;   /*读寄存器失败*/
	}

	/*计算接收的前14个字节的校验和，并与接收的第15个CRC字节比较。*/
  if(scrb[sizeof(MDC04_SCRPARAMETERS)-1] != MY_OW_CRC8(scrb, sizeof(MDC04_SCRPARAMETERS)-1))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	scr->Cfb &= ~CFB_CFBSEL_Mask;
	scr->Cfb |= Cfb;

	MDC04_WriteParameters_SkipRom(scrb);

  return TRUE;
}

/**
  * @brief  读电容转换通道选择
  * @param  chann：通道选择寄存器Ch_Sel低3位的内容，可能为：
		CCS_CapChannel_Cap1
		CCS_CapChannel_Cap2
		CCS_CapChannel_Cap3
		CCS_CapChannel_Cap4
		CCS_CapChannel_Cap1_2
		CCS_CapChannel_Cap1_2_3
		CCS_CapChannel_Cap1_2_3_4
  * @retval 状态
*/
bool GetCapChannel(uint8_t *chann)
{
	uint8_t scrb[sizeof(MDC04_SCRPARAMETERS)];
	MDC04_SCRPARAMETERS *scr = (MDC04_SCRPARAMETERS *) scrb;

	/*读15个字节。第4字节是通道选择寄存器，最后字节是前14个的校验和--CRC。*/
	if(MDC04_ReadParameters_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前14个字节的校验和，并与接收的第15个CRC字节比较。*/
  if(scrb[sizeof(MDC04_SCRPARAMETERS)-1] != MY_OW_CRC8(scrb, sizeof(MDC04_SCRPARAMETERS)-1))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	*chann = scr->Ch_Sel & CCS_CHANNEL_Mask;

	return TRUE;
}

/**
  * @brief  写电容转换通道选择
  * @param  chann：通道选择寄存器Ch_Sel低3位的内容，可能为：
		CCS_CapChannel_Cap1
		CCS_CapChannel_Cap2
		CCS_CapChannel_Cap3
		CCS_CapChannel_Cap4
		CCS_CapChannel_Cap1_2
		CCS_CapChannel_Cap1_2_3
		CCS_CapChannel_Cap1_2_3_4
  * @retval 状态
*/
bool SetCapChannel(uint8_t chann)
{
	uint8_t scrb[sizeof(MDC04_SCRPARAMETERS)];
	MDC04_SCRPARAMETERS *scr = (MDC04_SCRPARAMETERS *) scrb;

	/*读15个字节。第4字节是通道选择寄存器，最后字节是前14个的校验和--CRC。*/
	if(MDC04_ReadParameters_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读寄存器失败*/
	}

	/*计算接收的前14个字节的校验和，并与接收的第15个CRC字节比较。*/
  if(scrb[sizeof(MDC04_SCRPARAMETERS)-1] != MY_OW_CRC8(scrb, sizeof(MDC04_SCRPARAMETERS)-1))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	scr->Ch_Sel = (scr->Ch_Sel & ~CCS_CHANNEL_Mask) | (chann & CCS_CHANNEL_Mask);

	MDC04_WriteParameters_SkipRom(scrb);

	return TRUE;
}

/**
  * @brief  设置周期测量频率和重复性
  * @param  mps 要设置的周期测量频率（每秒测量次数），可能为下列其一
	*				@arg CFG_MPS_Single		：每执行ConvertTemp一次，启动一次温度测量
	*				@arg CFG_MPS_Half			：每执行ConvertTemp一次，启动每秒0.5次重复测量
	*				@arg CFG_MPS_1				：每执行ConvertTemp一次，启动每秒1次重复测量
	*				@arg CFG_MPS_2				：每执行ConvertTemp一次，启动每秒2次重复测量
	*				@arg CFG_MPS_4				：每执行ConvertTemp一次，启动每秒4次重复测量
	*				@arg CFG_MPS_10				：每执行ConvertTemp一次，启动每秒10次重复测量
  * @param  repeatability：要设置的重复性值，可能为下列其一
	*				@arg CFG_Repeatbility_Low				：设置低重复性
	*				@arg CFG_Repeatbility_Medium		：设置中重复性
	*				@arg CFG_Repeatbility_High			：设置高重复性
  * @retval 无
*/
bool SetConfig(uint8_t mps, uint8_t repeatability)
{
	uint8_t scrb[sizeof(MDC04_SCRATCHPAD_READ)];
	MDC04_SCRATCHPAD_READ *scr = (MDC04_SCRATCHPAD_READ *) scrb;

	/*读9个字节。第7字节是系统配置寄存器，第8字节是系统状态寄存器。最后字节是前8个的校验和--CRC。*/
	if(MDC04_ReadScratchpad_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*读暂存器组水平*/
	}

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
  if(scrb[8] != MY_OW_CRC8(scrb, 8))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	scr->Cfg &= ~CFG_Repeatbility_Mask;
	scr->Cfg |= repeatability;
	scr->Cfg &= ~CFG_MPS_Mask;
	scr->Cfg |= mps;

	MDC04_WriteScratchpad_SkipRom(scrb+4);

	return TRUE;
}

/**
  * @brief  读状态和配置
  * @param  status 返回的状态寄存器值
  * @param  cfg 返回的配置寄存器值
  * @retval 状态
*/
bool ReadStatusConfig(uint8_t *status, uint8_t *cfg)
{
	uint8_t scrb[sizeof(MDC04_SCRATCHPAD_READ)];
	MDC04_SCRATCHPAD_READ *scr = (MDC04_SCRATCHPAD_READ *) scrb;

	/*读9个字节。第7字节是系统配置寄存器，第8字节是系统状态寄存器。最后字节是前8个的校验和--CRC。*/
	if(MDC04_ReadScratchpad_SkipRom(scrb) == FALSE)
	{
		return FALSE;  /*CRC验证未通过*/
	}

	/*计算接收的前8个字节的校验和，并与接收的第9个CRC字节比较。*/
  if(scrb[8] != MY_OW_CRC8(scrb, 8))
  {
		return FALSE;  /*CRC验证未通过*/
  }

	*status = scr->Status;
	*cfg = scr->Cfg;

	return TRUE;
}

/**
  * @brief  将偏置电容数值（pF）转换为对应的偏置电容配置
  * @param  osCap：偏置电容的数值
  * @retval 对应偏置配置寄存器的数值
*/
uint8_t CaptoCoscfg(float osCap)
{int i; uint8_t CosCfg = 0x00;

	for(i = 7; i >= 0; i--)
	{
		if(osCap >= COS_Factor[i])
		{
			CosCfg |= (0x01 << i);
			osCap -= COS_Factor[i];
		}
	}

	return CosCfg;
}

/**
  * @brief  将偏置电容配置转换为对应的偏置电容数值（pF）
  * @param  osCfg：偏置电容配置
  * @retval 对应偏置电容的数值
*/
float CoscfgtoCapOffset(uint8_t osCfg)
{
	uint8_t i;
	float Coffset = 0.0;

	for(i = 0; i < 8; i++)
	{
		if(osCfg & 0x01) Coffset += COS_Factor[i];
		osCfg >>= 1;
	}

	return Coffset;
}

/**
  * @brief  将量程电容数值（pF）转换为对应的量程电容配置
  * @param  fsCap：量程电容的数值
  * @retval 对应量程配置的数值
*/
uint8_t CapRangetocfbCfg(float fsCap)
{int8_t i; uint8_t CfbCfg = 0x00;

	fsCap = fsCap * (3.6/0.507);

	fsCap -= CFB.Cfb0;

	for(i = 5; i >= 0; i--)
	{
		if(fsCap >= CFB.Factor[i])
		{
			fsCap -= CFB.Factor[i];
			CfbCfg |= (0x01 << i);
		}
	}

	return CfbCfg;
}

/**
  * @brief  将量程电容配置转换为对应的量程电容数值（pF）
  * @param  fbCfg：量程电容配置
  * @retval 对应量程电容的数值
*/
float CfbcfgtoCapRange(uint8_t fbCfg)
{
	uint8_t i;
	float Crange = CFB.Cfb0;

	for(i = 0; i <= 5; i++)
	{
		if(fbCfg & 0x01) Crange += CFB.Factor[i];
		fbCfg >>= 1;
	}

	return (0.507/3.6) * Crange;
}

/**
  * @brief  获取配置的偏置电容数值（pF）
  * @param  Coffset：偏置电容配置
  * @retval 无
*/
void GetCfg_CapOffset(float *Coffset)
{uint8_t Cos_cfg;

	ReadCosConfig(&Cos_cfg);
	*Coffset = CoscfgtoCapOffset(Cos_cfg);
}

/**
  * @brief  获取配置的量程电容数值（pF）
  * @param  Crange：返回量程电容数值
  * @retval 无
*/
void GetCfg_CapRange(float *Crange)
{
	uint8_t Cfb_cfg;

	ReadCfbConfig(&Cfb_cfg);
	*Crange = CfbcfgtoCapRange(Cfb_cfg);
}

/**
  * @brief  配置偏置电容
  * @param  Coffset：要配置的偏置电容数值。范围0~103.5 pF。
  * @retval 状态
*/
bool MDC04_CapConfigureOffset(float Coffset)
{
	uint8_t CosCfg, Cosbits;
	CosCfg = CaptoCoscfg(Coffset + 0.25);

	if(!(CosCfg & ~0x1F)) Cosbits = COS_RANGE_5BIT;
	else if(!(CosCfg & ~0x3F)) Cosbits = COS_RANGE_6BIT;
			 else if(!(CosCfg & ~0x7F)) Cosbits = COS_RANGE_7BIT;
						else Cosbits = COS_RANGE_8BIT;

	WriteCosConfig(CosCfg, Cosbits);

	return TRUE;
}

/**
  * @brief  配置量程电容
  * @param  Cfs：要配置的量程电容数值。范围+/-（0.281~15.49） pF。
  * @retval 状态
*/
bool MDC04_CapConfigureFs(float Cfs)
{
	uint8_t Cfbcfg;

	Cfs = (Cfs + 0.1408);
	Cfbcfg = CapRangetocfbCfg(Cfs);

	WriteCfbConfig(Cfbcfg);

	return TRUE;
}

/**
  * @brief  配置电容测量范围
  * @param  Cmin：要配置测量范围的低端。
  * @param  Cmax：要配置测量范围的高端。
  * @retval 状态
*/
bool MDC04_CapConfigureRange(float Cmin, float Cmax)
{ float Cfs, Cos;

//	if(!((Cmax <= 119.0) && (Cmax > Cmin) && (Cmin >= 0.0) && ((Cmax-Cmin) <= 31.0)))
//	return FALSE;	//The input value is out of range.

	Cos = (Cmin + Cmax)/2.0;
	Cfs = (Cmax - Cmin)/2.0;

	MDC04_CapConfigureOffset(Cos);
	MDC04_CapConfigureFs(Cfs);

	return TRUE;
}


/**
  * @brief  读电容配置
  * @param  Coffset：配置的偏置电容。
  * @param  Crange：配置的量程电容。
  * @retval 无
*/
bool ReadCapConfigure(float *Coffset, float *Crange)
{
	GetCfg_CapOffset(Coffset);
	GetCfg_CapRange(Crange);

	return TRUE;
}


