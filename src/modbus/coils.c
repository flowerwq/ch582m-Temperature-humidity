#include <stdio.h>
#include <string.h>
#include "stdint.h"
#include "modbus.h"
#include "utils.h"
typedef struct mb_coils_ctx{
	uint16_t start;
	uint16_t len;
	uint8_t *content;
} mb_coils_ctx_t;


mb_coils_ctx_t mb_coils[] = {
};


void mb_coils_init(){
	int i;
	for (i = 0; i < sizeof(mb_coils)/sizeof(mb_coils_ctx_t); i++){
		memset(mb_coils[i].content, 0, (mb_coils[i].len + 7) / 8);
	}
}

static uint16_t modbus_coil_read(uint16_t index){
	uint16_t i, idx;
	for (i = 0; i < sizeof(mb_coils)/sizeof(mb_coils_ctx_t); i++){
		if(index >= mb_coils[i].start && 
			index < mb_coils[i].start + mb_coils[i].len)
		{
			idx = index - mb_coils[i].start;
			return (mb_coils[i].content[idx/8] >> (idx % 8)) & 0x01;
		}
	}
	return 0;
}

static void modbus_coil_write(uint16_t index, uint16_t value){
	uint16_t i, idx, mask;
	for (i = 0; i < sizeof(mb_coils)/sizeof(mb_coils_ctx_t); i++){
		if(index >= mb_coils[i].start && 
			index < mb_coils[i].start + mb_coils[i].len)
		{
			idx = index - mb_coils[i].start;
			mask = 0x01 << (idx % 8);
			idx /= 8;
			if (value){
				 mb_coils[i].content[idx] = BITS_SET(mb_coils[i].content[idx], mask);
			}else{
				mb_coils[i].content[idx] = BITS_CLEAR(mb_coils[i].content[idx], mask);
			}
		}
	}
}

static uint16_t modbus_coil_r_check(uint16_t index){
	int i;
	for (i = 0; i < sizeof(mb_coils)/sizeof(mb_coils_ctx_t); i++){
		if(index >= mb_coils[i].start && 
			index < mb_coils[i].start + mb_coils[i].len)
		{
			return 1;
		}
	}
	return 0;
}

static uint16_t modbus_coil_w_check(uint16_t index){
	int i;
	for (i = 0; i < sizeof(mb_coils)/sizeof(mb_coils_ctx_t); i++){
		if(index >= mb_coils[i].start && 
			index < mb_coils[i].start + mb_coils[i].len)
		{
			return 1;
		}
	}
	return 0;
}

void modbus_coil_update(mb_coil_addr_t addr, uint8_t value){
	uint16_t i, idx, mask;
	for (i = 0; i < sizeof(mb_coils)/sizeof(mb_coils_ctx_t); i++){
		if(addr >= mb_coils[i].start && 
			addr < mb_coils[i].start + mb_coils[i].len)
		{
			idx = addr - mb_coils[i].start;
			mask = 0x01 << (idx % 8);
			idx /= 8;
			if (value){
				mb_coils[i].content[idx] = 
					BITS_SET(mb_coils[i].content[idx], mask);
			}else{
				mb_coils[i].content[idx] = 
					BITS_CLEAR(mb_coils[i].content[idx], mask);
			}
		}
	}
}

ModbusError modbus_coil_callback( void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out)
{
	mb_slave_ctx_t *sctx = (mb_slave_ctx_t *)ctx;
	switch(args->query){
	case MODBUS_REGQ_R_CHECK:
		if (!modbus_coil_r_check(args->index)){
			return MODBUS_ERROR_INDEX;
		}
		break;
	case MODBUS_REGQ_R:
		out->value = modbus_coil_read(args->index);
		break;
	case MODBUS_REGQ_W_CHECK:
		if (!modbus_coil_w_check(args->index)){
			return MODBUS_ERROR_INDEX;
		}
		break;
	case MODBUS_REGQ_W:
		if (MODBUS_COIL == args->type){
			if (sctx && sctx->callback.before_coil_write){
				if (sctx->callback.before_coil_write(args->index, args->value)){
					break;
				}
			}
			modbus_coil_write(args->index, args->value);
			if (sctx && sctx->callback.after_coil_write){
				sctx->callback.after_coil_write(args->index, args->value);
			}
		}
		break;
	}
	return 0;
}

