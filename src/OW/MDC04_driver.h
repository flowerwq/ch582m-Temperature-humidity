/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MDC04_driver_H
#define MDC04_driver_H

//*********MCU文件**********//
#include "stm32f10x_rcc.h"
#include "MY_stdtype.h"

/* MDC04/01 Registers definition----------------------------------------------*/
/*Bit definition of config register*/
#define CFG_CLKSTRETCH_Mask   	0x20
#define CFG_MPS_Mask   		  	0x1C
#define CFG_Repeatbility_Mask 	0x03

#define CFG_MPS_Single  	0x00
#define CFG_MPS_Half  		0x04
#define CFG_MPS_1  			0x08
#define CFG_MPS_2 			0x0C
#define CFG_MPS_4 			0x10
#define CFG_MPS_10 			0x14

#define CFG_Repeatbility_Low   	0x00
#define CFG_Repeatbility_Medium 0x01
#define CFG_Repeatbility_High 	0x02

#define CFG_ClkStreatch_Disable (0x00 << 5)
#define CFG_ClkStreatch_Enable 	(0x01 << 5)
/*Bit definition of status register*/
#define Status_Meature_Mask   	0x81
#define Status_WriteCrc_Mask   	0x20
#define Status_CMD_Mask   			0x10
#define Status_POR_Mask   			0x08

/*Bit definition of CFB register*/
#define CFB_COSRANGE_Mask   		0xC0
#define CFB_CFBSEL_Mask   			0x3F

#define CFB_COS_BITRANGE_5  		0x1F
#define CFB_COS_BITRANGE_6  		0x3F
#define CFB_COS_BITRANGE_7  		0x7F
#define CFB_COS_BITRANGE_8  		0xFF

#define COS_RANGE_5BIT				  		0x00
#define COS_RANGE_6BIT				  		0x40
#define COS_RANGE_7BIT				  		0x80
#define COS_RANGE_8BIT				  		0xC0
/*Bit definition of Ch_Sel register*/
#define CCS_CHANNEL_Mask   					0x07

#define CCS_CapChannel_Cap1					0x01 		
#define CCS_CapChannel_Cap2					0x02
#define CCS_CapChannel_Cap3					0x03
#define CCS_CapChannel_Cap4					0x04
#define CCS_CapChannel_Cap1_2				0x05
#define CCS_CapChannel_Cap1_2_3			    0x06
#define CCS_CapChannel_Cap1_2_3_4		    0x07

#define CAP_CH1_SEL  								0x01
#define CAP_CH2_SEL  								0x02
#define CAP_CH3_SEL  								0x03
#define CAP_CH4_SEL  								0x04
#define CAP_CH1CH2_SEL  						0x05
#define CAP_CH1CH2CH3_SEL  					0x06
#define CAP_CH1CH2CH3CH4_SEL  			0x07

/******************  Bit definition for MDC04 configuration register  ******************/
#define MDC04_CFG_REPEATABILITY_MASK          0x03
#define MDC04_CFG_MPS_MASK										0x1C
#define MDC04_CFG_I2CCLKSTRETCH_MASK          0x20
/******************  Bit definition for MDC04 temperature register  *******/
#define MDC04_REPEATABILITY_LOW               (0x00 << 0)
#define MDC04_REPEATABILITY_MEDIUM            (0x01 << 0)
#define MDC04_REPEATABILITY_HIGH              (0x02 << 0)
/******************  Bit definition for TTrim in parameters  *******/
#define MDC04_MPS_SINGLE					            (0x00 << 2)
#define MDC04_MPS_0P5Hz					            	(0x01 << 2)
#define MDC04_MPS_1Hz					            		(0x02 << 2)
#define MDC04_MPS_2Hz					            		(0x03 << 2)
#define MDC04_MPS_4Hz					            		(0x04 << 2)
#define MDC04_MPS_10Hz					            	(0x05 << 2)

#define MDC04_CLKSTRETCH_EN					          (0x01 << 5)
/******************  Bit definition for status register  *******/
#define MDC04_STATUS_CONVERTMODE_MASK          0x81
#define MDC04_STATUS_I2CDATACRC_MASK           0x20
#define MDC04_STATUS_I2CCMDCRC_MASK            0x10
#define MDC04_STATUS_SYSRESETFLAG_MASK         0x08

#define MDC04_CONVERTMODE_IDLE             		 0x00
#define MDC04_CONVERTMODE_T             		   0x01
#define MDC04_CONVERTMODE_C             		   0x02
#define MDC04_CONVERTMODE_TC1            		   0x03
/******************  Bit definition for channel select register  *******/
#define MDC04_CHANNEl_SELECT_MASK           	 0x07
#define MDC04_CHANNEl_C1           					   0x01
#define MDC04_CHANNEl_C2           					   0x02
#define MDC04_CHANNEl_C3           					   0x03
#define MDC04_CHANNEl_C4           					   0x04
#define MDC04_CHANNEl_C1C2           				   0x05
#define MDC04_CHANNEl_C1C2C3           		   	 0x06
#define MDC04_CHANNEl_C1C2C3C4           	   	 0x07
/******************  Bit definition for feeadback capacitor register  *******/
#define MDC04_CFEED_OSR_MASK           	   		 0xC0
#define MDC04_CFEED_CFB_MASK           	   		 0x3F

/* MDC04/01 ow Commands-------------------------------------------------------*/
typedef enum
{
	//ROM command
    SKIP_ROM            	= 0xcc,
    READ_ROM            	= 0x33,
    MATCH_ROM           	= 0x55,
	SEARCH_ROM           	= 0xf0, 
	ALARM_SEARCH			= 0xec,
	//Function command
    CONVERT_C           	= 0x66,
    CONVERT_T           	= 0x44,
	CONVERT_TC1             = 0x10,	
	READ_SCRATCHPAD     	= 0xbe,
	WRITE_SCRATCHPAD     	= 0x4e,
	READ_TC1             	= 0xcf,	
	READ_C2C3C4			    = 0xdc,
	READ_PARAMETERS      	= 0x8b,
	WRITE_PARAMETERS     	= 0xab,	
	COPY_PAGE0				= 0x48,
	READ_SCRATCHPAD_EXT  	= 0xdd,
	WRITE_SCRATCHPAD_EXT 	= 0x77,
} MDC04_OW_CMD;

/******************  Scratchpad/SRAM  ******************/
/*SRAM scratchpad*/
typedef struct
{
	uint8_t T_lsb;					/*The LSB of 温度结果, RO*/
	uint8_t T_msb;					/*The MSB of 温度结果, RO*/
	uint8_t C1_lsb;					/*The LSB of 电容通道C1, RO*/
	uint8_t C1_msb;					/*The MSB of 电容通道C1, Ro*/	
	uint8_t Tha_set_lsb;		
	uint8_t Tla_set_lsb;		
	uint8_t Cfg;						/*系统配置寄存器, RW*/
	uint8_t Status;					/*系统状态寄存器, RO*/
	uint8_t crc_scr;				/*CRC for byte0-7, RO*/
} MDC04_SCRATCHPAD_READ;

typedef struct
{	
	int8_t Tha_set_lsb;				
	int8_t Tla_set_lsb;			
	uint8_t Cfg;						/*系统配置寄存器, RW*/
} MDC04_SCRATCHPAD_WRITE;

typedef struct
{
	uint8_t tha_clear;				
	uint8_t tla_clear;					
	uint8_t hha_set;					
	uint8_t hla_set;					
	uint8_t hha_clear;				
	uint8_t hla_clear;					
	uint8_t udf[5];							
	uint8_t MPW_test;					
	uint8_t crc_ext;						
} MDC04_SCRATCHPADEXT;

typedef struct
{	
	uint8_t C2_lsb;				/*The LSB of C2, RO*/
	uint8_t C2_msb;				/*The MSB of C2, RO*/
	uint8_t C3_lsb;				/*The LSB of C3, RO*/
	uint8_t C3_msb;				/*The MSB of C3, RO*/
	uint8_t C4_lsb;				/*The LSB of C4, RO*/
	uint8_t C4_msb;				/*The MSB of C4, RO*/
/*crc*/	
} MDC04_C2C3C4;

typedef struct
{
	uint8_t Family;				/*Family byte, RO*/
	uint8_t Id[6];				/*Unique ID, RO*/
	uint8_t crc_rc;				/*Crc code for byte0-7, RO*/
} MDC04_ROMCODE;

typedef struct
{
	uint8_t Res[3];
	uint8_t Ch_Sel;					/*电容通道选择寄存器，RW*/
	uint8_t Cos;						/*偏置电容配置寄存器，RW*/
	uint8_t Res1;				
	uint8_t T_coeff[3];			
	uint8_t Cfb;						/*量程电容配置寄存器，RW*/									
	uint8_t Res2;
	uint8_t Res3[2];
	uint8_t dummy8;
	uint8_t crc_para;				/*CRC for byte0-13, RO*/
} MDC04_SCRPARAMETERS;


/*顶层驱动函数原型*/
int MY_Read_ROM(void);
int MY_T(void);
int MY_TC1(void);
int MY_C(void);
int MY_P(void);
int MY_F(int repeatability,int mps);
int MY_Channel(uint8_t channel);
int MY_Offset(float Co);
int MY_FullScale(float Cr);
int MY_Range(float Cmin,float Cmax);
int MY_EEPROM(void);

/*底层函数原型*/
float MDC04_OutputtoTemp(int16_t out);
int16_t MDC04_TemptoOutput(float Temp);
bool MDC04_ReadROM(uint8_t *scr);
float MDC04_OutputtoCap(uint16_t out, float Co, float Cr);
uint8_t MY_OW_CRC8(uint8_t *serial, uint8_t length);
bool MDC04_nReadScratchpad_SkipRom(uint8_t *scr, uint8_t size);
bool MDC04_ReadScratchpad_SkipRom(uint8_t *scr);
bool MDC04_WriteScratchpad_SkipRom(uint8_t *scr);
bool MDC04_ReadScratchpadExt_SkipRom(uint8_t *scr);
bool MDC04_WriteScratchpadExt_SkipRom(uint8_t *scr);
bool MDC04_ReadC2C3C4_SkipRom(uint8_t *scr);
bool MDC04_ReadParameters_SkipRom(uint8_t *scr);
bool MDC04_WriteParameters_SkipRom(uint8_t *scr);
bool SavetoE2PROMPage0(void);
bool ConvertTemp(void);
bool ConvertTC1(void);
bool ConvertCap(void);
bool ReadTempWaiting(uint16_t *iTemp);
bool ReadTempPolling(uint16_t *iTemp);
bool ReadTempCap1(uint16_t *iTemp, uint16_t *iCap1);
bool ReadTempCap1Polling(uint16_t *iTemp, uint16_t *iCap1);
bool ReadCapC2C3C4(uint16_t *iCap);
bool ReadCosConfig(uint8_t *Coscfg);
bool WriteCosConfig(uint8_t Coffset, uint8_t Cosbits);
bool ReadCfbConfig(uint8_t *Cfb);
bool WriteCfbConfig(uint8_t Cfb);
bool GetCapChannel(uint8_t *chann);
bool SetCapChannel(uint8_t chann);
bool SetConfig(uint8_t mps, uint8_t repeatability);
bool ReadStatusConfig(uint8_t *status, uint8_t *cfg);
uint8_t CaptoCoscfg(float osCap);
float CoscfgtoCapOffset(uint8_t osCfg);
uint8_t CapRangetocfbCfg(float fsCap);
float CfbcfgtoCapRange(uint8_t fbCfg);
void GetCfg_CapOffset(float *Coffset);
void GetCfg_CapRange(float *Crange);
bool MDC04_CapConfigureOffset(float Coffset);
bool MDC04_CapConfigureFs(float Cfs);
bool MDC04_CapConfigureRange(float Cmin, float Cmax);
bool ReadCapConfigure(float *Coffset, float *Crange);

#endif /*MDC04_driver_H */
