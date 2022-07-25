#include <stdio.h>
#include <string.h>
#include "modbus.h"

#include "utils.h"
#include "version.h"

static uint16_t mb_ro_regs[MB_REG_ADDR_RO_MAX - MB_REG_ADDR_RO_BASE];
static uint16_t mb_config_regs[MB_REG_ADDR_CONFIG_MAX - MB_REG_ADDR_CONFIG_BASE];
static uint16_t mb_warn_rcd_regs[MB_REG_ADDR_WARN_RCD_MAX - MB_REG_ADDR_WARN_RCD_BASE];
static uint16_t mb_rwa_regs[MB_REG_ADDR_RWA_MAX - MB_REG_ADDR_RWA_BASE];
static uint16_t mb_test_regs[MB_REG_ADDR_TEST_MAX - MB_REG_ADDR_TEST_BASE];

static mb_reg_segment_t mb_reg_segs[] = {
	{
		{.reg_type = MB_REG_T_HR, .access_perm = MB_REG_PERM_R}, 
		MB_REG_ADDR_RO_BASE, 
		MB_REG_ADDR_RO_MAX - MB_REG_ADDR_RO_BASE,
		mb_ro_regs
	},
	{
		{.reg_type = MB_REG_T_HR, .access_perm = MB_REG_PERM_RW}, 
		MB_REG_ADDR_CONFIG_BASE, 
		MB_REG_ADDR_CONFIG_MAX - MB_REG_ADDR_CONFIG_BASE,
		mb_config_regs
	},
	{
		{.reg_type = MB_REG_T_HR, .access_perm = MB_REG_PERM_RWA}, 
		MB_REG_ADDR_RWA_BASE, 
		MB_REG_ADDR_RWA_MAX - MB_REG_ADDR_RWA_BASE,
		mb_rwa_regs
	},
	{
		{.reg_type = MB_REG_T_HR, .access_perm = MB_REG_PERM_RW}, 
		MB_REG_ADDR_TEST_BASE, 
		MB_REG_ADDR_TEST_MAX - MB_REG_ADDR_TEST_BASE,
		mb_test_regs
	},
};

/**
 *@desc update register
 *
 */
void modbus_reg_update(mb_reg_addr_t addr, uint16_t value)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (addr >= mb_reg_segs[i].addr_start && 
			addr < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			mb_reg_segs[i].content[addr - mb_reg_segs[i].addr_start] = value;
		}
	}
}

uint16_t modbus_reg_get(mb_reg_addr_t addr)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (addr >= mb_reg_segs[i].addr_start && 
			addr < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			return mb_reg_segs[i].content[addr - mb_reg_segs[i].addr_start];
		}
	}
	return 0;
}

/*
 *@desc 获取modbus寄存器实际内存地址，仅用于读取数据，注意越界
 */
uint16_t *modbus_reg_buf_addr(mb_reg_addr_t addr){
	int i = 0;
	int reg_idx = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (addr >= mb_reg_segs[i].addr_start && 
			addr < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			reg_idx = addr - mb_reg_segs[i].addr_start;
			return &mb_reg_segs[i].content[reg_idx];
		}
	}

	return NULL;
}

static int8_t modbus_reg_write(mb_reg_addr_t addr, uint16_t value)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (addr >= mb_reg_segs[i].addr_start && 
			addr < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			mb_reg_segs[i].content[addr - mb_reg_segs[i].addr_start] = value;
		}
	}
	return 0;
}


static uint16_t modbus_reg_read(mb_reg_addr_t addr)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (addr >= mb_reg_segs[i].addr_start && 
			addr < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			return mb_reg_segs[i].content[addr - mb_reg_segs[i].addr_start];
		}
	}
	return 0;
}

static uint16_t modbus_reg_w_check(uint16_t index, mb_slave_ctx_t *sctx){
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (index >= mb_reg_segs[i].addr_start && 
			index < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			switch(mb_reg_segs[i].type.access_perm){
				case MB_REG_PERM_R:
					return 0;
				case MB_REG_PERM_RW:
					return 1;
				case MB_REG_PERM_RWA:
					return sctx->flag_safe_access;
				default:
					return 0;
			}
		}
	}
	return 0;
}

static uint16_t modbus_reg_r_check(uint16_t index, mb_slave_ctx_t *sctx){
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		if (index >= mb_reg_segs[i].addr_start && 
			index < mb_reg_segs[i].addr_start + mb_reg_segs[i].len)
		{
			return 1;
		}
	}
	return 0;
}

ModbusError modbus_reg_callback(void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out)
{
	mb_slave_ctx_t *sctx = (mb_slave_ctx_t *)ctx;
	out->exceptionCode = MODBUS_EXCEP_NONE;
	switch(args->query){
	case MODBUS_REGQ_R_CHECK:
		if (MODBUS_HOLDING_REGISTER == args->type){
			if(!modbus_reg_r_check(args->index, sctx)){
				return MODBUS_ERROR_INDEX;
			}
			return MODBUS_OK;
		}else{
			return MODBUS_ERROR_FUNCTION;
		}
		break;
	case MODBUS_REGQ_R:
		if (MODBUS_HOLDING_REGISTER == args->type){
			out->value = modbus_reg_read(args->index);
		}else{
			return MODBUS_ERROR_FUNCTION;
		}
		break;
	case MODBUS_REGQ_W_CHECK:
		if (MODBUS_HOLDING_REGISTER == args->type){
			if (!modbus_reg_w_check(args->index, sctx)){
				return MODBUS_ERROR_INDEX;
			}
			if (sctx && sctx->callback.before_reg_write){
				if (sctx->callback.before_reg_write(args->index, args->value)){
					out->exceptionCode = MODBUS_EXCEP_ILLEGAL_VALUE;
					return MODBUS_ERROR_VALUE;
					break;
				}
			}
			return MODBUS_OK;
		}else{
			return MODBUS_ERROR_FUNCTION;
		}
		break;
	case MODBUS_REGQ_W:
		if (MODBUS_HOLDING_REGISTER == args->type){
			if (sctx && sctx->callback.before_reg_write)
			{
				if (sctx->callback.before_reg_write(args->index, args->value)){
					break;
				}
			}
			
			if (0 != modbus_reg_write(args->index, args->value)){
				break;
			}
			if (sctx && sctx->callback.after_reg_write){
				sctx->callback.after_reg_write(args->index, args->value);
			}
		}else{
			return MODBUS_ERROR_FUNCTION;
		}
		break;
	}
	return 0;
}

void modbus_reg_update_uid(const uint8_t *uid, uint16_t len){
	uint16_t *buf = modbus_reg_buf_addr(MB_REG_ADDR_UID_7);
	int i = 0;
	uint16_t maxlen = MB_REG_ADDR_UID_0 - MB_REG_ADDR_UID_7 + 1;
	uint16_t bytes_remain = len;
	if (len <= 0){
		return;
	}
	for(i = 0; i < maxlen; i++){
		buf[i] = uid[i * 2] << 8;
		bytes_remain --;
		if (!bytes_remain){
			break;
		}
		buf[i] += uid[i * 2 + 1];
		bytes_remain --;
		if (!bytes_remain){
			break;
		}
	}
}

void modbus_regs_init(){
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(mb_reg_segs); i++){
		memset(mb_reg_segs[i].content, 0, 2 * mb_reg_segs[i].len);
	}
}

