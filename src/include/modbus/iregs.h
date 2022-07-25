#ifndef __MODBUS_IREGS_H__
#define __MODBUS_IREGS_H__
#include "stdint.h"
#include "lightmodbus/lightmodbus.h"

typedef enum mb_ireg_addr{
    MB_IREG_ADDR_TH_BASE = 0,
    MB_IREG_ADDR_TH_T= MB_IREG_ADDR_TH_BASE,
    MB_IREG_ADDR_TH_H,
    MB_IREG_ADDR_TH_MAX,
} mb_ireg_addr_t;

void modbus_ireg_update(mb_ireg_addr_t addr, uint16_t value);
void modbus_iregs_init();
ModbusError modbus_ireg_callback(void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out);

#endif
