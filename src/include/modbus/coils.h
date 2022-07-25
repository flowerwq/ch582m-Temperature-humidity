#ifndef __MODBUS_COILS_H__
#define __MODBUS_COILS_H__
#include "stdint.h"
#include "lightmodbus/slave.h"

typedef enum mb_coil_addr{
	MB_COILS_ADDR_BASE = 0,
	MB_COILS_ADDR_MAX
} mb_coil_addr_t;

void modbus_coil_update(mb_coil_addr_t addr, uint8_t value);
void mb_coils_init();
ModbusError modbus_coil_callback( void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out);

#endif
