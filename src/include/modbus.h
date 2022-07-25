#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "stdint.h"
#include "liblightmodbus.h"
#include "modbus/regs.h"
#include "modbus/iregs.h"
#include "modbus/coils.h"
#include "modbus/discrete_input.h"

/**
 * @desc: modbus register callback function
 * @param addr: register address
 * @param value: register value
 * @return: 0 on success，Nonzero value on failure.
 */
typedef int (*mb_reg_callback_t)(mb_reg_addr_t addr, uint16_t value);

/**
 * @desc: modbus coil callback function
 * @param addr: coil address
 * @param value: coil value, 0 means off, nonzero value means on
 * @return: 0 on success，Nonzero value on failure.
 */
typedef int (*mb_coil_callback_t)(mb_coil_addr_t addr, uint16_t value);

typedef struct mb_callback{
	mb_reg_callback_t before_reg_write;
	mb_reg_callback_t after_reg_write;
	mb_coil_callback_t before_coil_write;
	mb_coil_callback_t after_coil_write;
} mb_callback_t;

typedef enum modbus_status {
	MODBUS_S_INIT = 0,
	MODBUS_S_IDLE,
	MODBUS_S_RECV,
	MODBUS_S_CTRL_AND_WAITING,
	MODBUS_S_RECV_FINISH,
} modbus_status_t;

typedef struct modbus_slave_context{
	ModbusSlave slave;
	uint8_t address;
	uint32_t baudrate;
	uint32_t tcnt;
	uint8_t flag_frame_err;
	uint8_t flag_safe_access;
	uint64_t lasttime_sa;
	modbus_status_t status;
	mb_callback_t callback;
	int req_len;
	uint8_t req_buf[256];
	uint8_t resp_buf[256];
	uint32_t lasttime_recv;
} mb_slave_ctx_t;

int modbus_is_receiving();
void modbus_init(mb_callback_t *callback);
void modbus_deinit();
void modbus_frame_check();
void modbus_run();
int modbus_sa_ctrl(int enable);
void modbus_reload();

#endif
