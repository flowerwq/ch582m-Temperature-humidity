#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "modbus.h"

static uint16_t mb_iregs[MB_IREG_ADDR_TH_MAX - MB_IREG_ADDR_TH_BASE];

void modbus_ireg_update(mb_ireg_addr_t addr, uint16_t value)
{
	if (addr >= MB_IREG_ADDR_TH_BASE && addr < MB_IREG_ADDR_TH_MAX){
		mb_iregs[addr - MB_IREG_ADDR_TH_BASE] = value;
	}
}
/**
 *@return 1-allow read;0-read operation not allowed
 */
static uint16_t modbus_ireg_r_check(uint16_t index){
	if (index >= MB_IREG_ADDR_TH_BASE && index < MB_IREG_ADDR_TH_MAX)
	{
		return 1;
	}
	return 0;
}

static uint16_t modbus_ireg_read(uint16_t index){
	if (index >= MB_IREG_ADDR_TH_BASE && index < MB_IREG_ADDR_TH_MAX)
	{
		return mb_iregs[index - MB_IREG_ADDR_TH_BASE];
	}
	return 0;
}

ModbusError modbus_ireg_callback(void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out)
{
//	mb_slave_ctx_t *sctx = (mb_slave_ctx_t *)ctx;
	out->exceptionCode = MODBUS_EXCEP_NONE;
	switch(args->query){
		case MODBUS_REGQ_R_CHECK:
			if(!modbus_ireg_r_check(args->index)){
				return MODBUS_ERROR_INDEX;
			}
			break;
		case MODBUS_REGQ_R:
			out->value = modbus_ireg_read(args->index);
			break;
		default:
			return MODBUS_ERROR_INDEX;
	}
	return MODBUS_OK;
}

void modbus_iregs_init(){
	memset(mb_iregs, 0, sizeof(mb_iregs));
	
}



