/* Includes ------------------------------------------------------------------*/
#include "MY_ow.h"
#include "stm32f10x.h"
#include "bsp_SysTick.h"



/******One-wire communication timing requirements*******/
//SPON: <=10 us. Time to Strong Pullup On.
//tSLOT: >=60 us, <=120us. Time Slot
//tREC: >=1 us. Recovery Time.
//tLOW0: >=60 us, <=120us. Write 0 Low Time.
//tLOW1: >=1 us, <=15us. Write 1 Low Time.
//tRDV: <=15 us. Read Data Valid.
//tRSTH: >=480 us. Reset Time High.
//tRSTL: >=480 us. Reset Time Low.
//PDHIGH: >=15 us, <=60us. Presence-Detect High.
//tPDLOW: >=60 us, <=240us. Presence-Detect Low.

/*Time delays in us for GPIO simulated master One-wire communication*/
#define 				tSlot										60				/*us*/
#define					tRecover								10				/*us*/
#define					tInitSlot								2					/*us*/
#define 				tLow_Write_1						tInitSlot
#define 				tHigh_Write_1						(tSlot+tRecover-tInitSlot)
#define 				tLow_Write_0						tSlot
#define 				tHigh_Write_0						tRecover
#define 				tLow_Read								tInitSlot
#define 				tSample_Read						15    		/*us*/
#define 				tComplement_Read				(tSlot+tRecover-tInitSlot-tSample_Read)
#define 				tLow_Reset							480				/*us*/
#define					tHigh_Reset							480				/*us*/
#define					tSample_Presence				70				/*us*/
#define					tComplement_Presence		(tHigh_Reset-tSample_Presence)
//---------------------------------------------------------------------------

void OW_Init(void)
{
	ow_DQ_init();
	ow_DQ_set();
}


// OW 'RESET+PRESENSE' timming sequence
//
//      |     >=480us   |        >=480us             |
//------                 --------\                ------------
//      |               /        |              /
//      | (>480us)	    |	15-60us|   60-240us   |
//      |_ _ _ _ _ _ _ _|        | _ _ _ _ _ _ _|

void OW_Reset(void)
{
	ow_DQ_reset(); 	// Drive DQ low
	ow_Delay_us(tLow_Reset);
	ow_DQ_set(); 		// Release DQ
}

bool OW_Presence(void)
{
	uint8_t dq;

	ow_Delay_us(tSample_Presence);
	dq = ow_DQ_get(); 						 // Get presence pulse from slave
	ow_Delay_us(tComplement_Presence); // Complete the reset sequence recovery

	return (dq ? FALSE : TRUE);
}

bool OW_ResetPresence(void)
{
	uint8_t dq;

	ow_DQ_reset(); 	// Drive DQ low
	ow_Delay_us(tLow_Reset);
	ow_DQ_set(); 		// Release DQ
	ow_Delay_us(tSample_Presence);
	dq = ow_DQ_get(); 									// Get presence pulse from slave
	ow_Delay_us(tComplement_Presence); 	// Complete the reset-presensce

	return (dq ? FALSE : TRUE);
}

//---------------------------------------------------------------------------
//   MASTER WRITE 0 and WRITE 1 SLOT=70us.
//   |>1us|       60<Tx0<120us         |1us<tRec|
//    ----                              ----------
//  /     |                            /
//  |     | 	      |     45us      |  |
//  |_ _ _|_ _ _ _______ _ _ _ _____ __|
//   			|   15us  | Slave samples |

//   |>1us|       60<Tx1<120us         |
//    ----       ---------------------------------
//  /     |     /                   |
//  |     |>1us	|   |     45us      |
//  |_ _ _|_ __ | _______ _ _ _ ___ |
//  		|   15us  | Slave samples |

// Send a bit to DQ. Provide 10us recovery time.
void OW_WriteBit(uint8_t bit)
{
	if (bit)
	{
		// Write '1' to DQ
		ow_DQ_reset(); 							// Initialte write '1' time slot.
		ow_Delay_us(tLow_Write_1);
		ow_DQ_set();
		ow_Delay_us(tHigh_Write_1); // Complete the write '1' time slot.
	}
	else
	{
		// Write '0' to DQ
		ow_DQ_reset();  						// Initialte write '0' time slot
		ow_Delay_us(tLow_Write_0);
		ow_DQ_set();
		ow_Delay_us(tHigh_Write_0); // Complete the write '0' time slot: recovery
	}
}

//---------------------------------------------------------------------------
// Read a bit from DQ. Provide 10us recovery time.
//
int OW_ReadBit(void)
{
	int bit;

	ow_DQ_reset(); 								// Initialte read time slot
	ow_Delay_us(tLow_Read);
	ow_DQ_set();
	ow_Delay_us(tSample_Read);
	bit = ow_DQ_get(); 				 		// Sample DQ to get the bit from the slave
	ow_Delay_us(tComplement_Read); // Complete the read time slot with 10us recovery

	return (bit != 0);
}

/*Send a byte to DQ. LSB first, MSB last.*/
void OW_WriteByte(uint8_t data)
{
	int bit;

	for (bit = 0; bit < 8; bit++)
	{
		OW_WriteBit(data & 0x01);
		data >>= 1;
	}
}
//---------------------------------------------------------------------------
// Read a byte from DQ and return it. LSB first, MSB last.
//
uint8_t OW_ReadByte(void)
{
	uint8_t bit, byte=0;

	for (bit = 0; bit < 8; bit++)
	{
		byte >>= 1;
		if (OW_ReadBit())
			byte |= 0x80;
	}

	return byte;
}

/* Single read time slot for polling slave ready.*/
OW_SLAVESTATUS OW_ReadStatus(void)
{
	int status;

	ow_DQ_reset(); 				         // Initiate read time slot
	ow_Delay_us(tLow_Read);
	ow_DQ_set();
	ow_Delay_us(tSample_Read);
	status = ow_DQ_get();          // Get the status from DQ: '0' busy, '1' idle.
	ow_Delay_us(tComplement_Read); // Complete the read time slot and recovery

	return (status ? READY : BUSY);
}
//---------------------------------------------------------------------------
//Multi-Drop 1-Wire network function: get a bit value and its complement.
//---------------------------------------------------------------------------
uint8_t OW_Read2Bits(void)
{
		uint8_t i, dq, data;
	  data = 0;

		for(i=0; i<2; i++)
		{
			dq = OW_ReadBit();
			data = (data) | (dq<<i);
		}

		return data;
}
//---------------------------------------------------------------------------
// CRC校验.
//

#define POLYNOMIAL 0x131 //100110001

uint8_t CRC8_Cal(uint8_t *serial, uint8_t length) 
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

