#ifndef __MB_DISCRETE_INPUT__
#define __MB_DISCRETE_INPUT__

typedef enum mb_di_addr{
	MB_DI_ADDR_BASE = 0,
	MB_DI_ADDR_MAX
} mb_di_addr_t;

void modbus_di_update(mb_di_addr_t addr, uint8_t value);
void mb_di_init();
ModbusError modbus_di_callback(void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out);

#endif
