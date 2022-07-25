#include <stdio.h>
#include <string.h>
#include "stdint.h"
#include "modbus.h"
#include "utils.h"

typedef struct mb_di_ctx{
	uint16_t start;
	uint16_t len;
	uint8_t *content;
} mb_di_ctx_t;

static uint8_t mb_di_cache[(MB_DI_ADDR_MAX - MB_DI_ADDR_BASE + 7)/8];

mb_di_ctx_t mb_di_list[] = {
	{MB_DI_ADDR_BASE, MB_DI_ADDR_MAX - MB_DI_ADDR_BASE, mb_di_cache},
};

void mb_di_init(){
	int i;
	for (i = 0; i < sizeof(mb_di_list)/sizeof(mb_di_ctx_t); i++){
		memset(mb_di_list[i].content, 0, (mb_di_list[i].len + 7) / 8);
	}
}

static uint16_t modbus_di_read(uint16_t index){
	uint16_t i, idx;
	for (i = 0; i < sizeof(mb_di_list)/sizeof(mb_di_ctx_t); i++){
		if(index >= mb_di_list[i].start && 
			index < mb_di_list[i].start + mb_di_list[i].len)
		{
			idx = index - mb_di_list[i].start;
			return (mb_di_list[i].content[idx/8] >> (idx % 8)) & 0x01;
		}
	}
	return 0;
}

static uint16_t modbus_di_r_check(uint16_t index){
	int i;
	for (i = 0; i < sizeof(mb_di_list)/sizeof(mb_di_ctx_t); i++){
		if(index >= mb_di_list[i].start && 
			index < mb_di_list[i].start + mb_di_list[i].len)
		{
			return 1;
		}
	}
	return 0;
}

void modbus_di_update(mb_di_addr_t addr, uint8_t value){
	uint16_t i, idx, mask;
	for (i = 0; i < sizeof(mb_di_list)/sizeof(mb_di_ctx_t); i++){
		if(addr >= mb_di_list[i].start && 
			addr < mb_di_list[i].start + mb_di_list[i].len)
		{
			idx = addr - mb_di_list[i].start;
			mask = 0x01 << (idx % 8);
			idx /= 8;
			if (value){
				mb_di_list[i].content[idx] = 
					BITS_SET(mb_di_list[i].content[idx], mask);
			}else{
				mb_di_list[i].content[idx] = 
					BITS_CLEAR(mb_di_list[i].content[idx], mask);
			}
		}
	}
}

ModbusError modbus_di_callback(void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out)
{
//	mb_slave_ctx_t *sctx = (mb_slave_ctx_t *)ctx;
	switch(args->query){
		case MODBUS_REGQ_R_CHECK:
			if(!modbus_di_r_check(args->index)){
				return MODBUS_ERROR_INDEX;
			}
			break;
		case MODBUS_REGQ_R:
			out->value = modbus_di_read(args->index);
			break;
		default:
			return MODBUS_ERROR_FUNCTION;
	}
	return 0;
}


