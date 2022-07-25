#ifndef __MODBUS_REGS_H__
#define __MODBUS_REGS_H__
#include "stdint.h"
#include "liblightmodbus.h"

#define MB_REG_CHANNEL_MAX 	32

#define MB_REG_APP_STATE_BOOT	1
#define MB_REG_APP_STATE_APP	2

#define MB_REG_DATA_BUF_SIZE	4096

typedef enum mb_reg_addr{
	MB_REG_ADDR_RO_BASE = 0,
	MB_REG_ADDR_APP_STATE = MB_REG_ADDR_RO_BASE,
	MB_REG_ADDR_VERSION_H,
	MB_REG_ADDR_VERSION_L,
	MB_REG_ADDR_HW_VERSION,
	MB_REG_ADDR_UID_7,
	MB_REG_ADDR_UID_6,
	MB_REG_ADDR_UID_5,
	MB_REG_ADDR_UID_4,
	MB_REG_ADDR_UID_3,
	MB_REG_ADDR_UID_2,
	MB_REG_ADDR_UID_1,
	MB_REG_ADDR_UID_0,
	MB_REG_ADDR_APP_VID,
	MB_REG_ADDR_APP_PID,
	MB_REG_ADDR_WORKTIME_H,
	MB_REG_ADDR_WORKTIME_L,
	MB_REG_ADDR_SEG_T_1,
	MB_REG_ADDR_SEG_ADDR_1,
	MB_REG_ADDR_SEG_LEN_1,
	MB_REG_ADDR_SEG_T_32 = MB_REG_ADDR_SEG_T_1 + (32 - 1) * 3,
	MB_REG_ADDR_SEG_ADDR_32,
	MB_REG_ADDR_SEG_LEN_32,
	MB_REG_ADDR_RO_MAX,
	
	MB_REG_ADDR_CONFIG_BASE = 128,
	MB_REG_ADDR_OPT_CTRL = MB_REG_ADDR_CONFIG_BASE,
	MB_REG_ADDR_WARN_EN,
	MB_REG_ADDR_WARN_OUTPUT_CTRL,
	MB_REG_ADDR_WARN_RESET_ID_H,
	MB_REG_ADDR_WARN_RESET_ID_L,
	MB_REG_ADDR_WARN_RESET,
	MB_REG_ADDR_SAFE_ACCESS_CTRL,
	MB_REG_ADDR_CONFIG_MAX,

	MB_REG_ADDR_WARN_RCD_BASE = 60000,
	MB_REG_ADDR_WARN_ID_H_0 = MB_REG_ADDR_WARN_RCD_BASE,
	MB_REG_ADDR_WARN_ID_L_0,
	MB_REG_ADDR_WARN_TS_H_0,
	MB_REG_ADDR_WARN_TS_L_0,
	MB_REG_ADDR_WARN_TYPE_0,
	MB_REG_ADDR_WARN_STATUS_0,

	MB_REG_ADDR_WRIN_ID_H_15 = MB_REG_ADDR_WARN_ID_H_0 + 15 * 6,
	MB_REG_ADDR_WARN_ID_L_15,
	MB_REG_ADDR_WARN_TS_H_15,
	MB_REG_ADDR_WARN_TS_L_15,
	MB_REG_ADDR_WARN_TYPE_15,
	MB_REG_ADDR_WARN_STATUS_15,
	MB_REG_ADDR_WARN_RCD_MAX,

	MB_REG_ADDR_RWA_BASE = 64000,
	MB_REG_ADDR_MB_ADDR	= MB_REG_ADDR_RWA_BASE,
	MB_REG_ADDR_BAUDRATE_H,
	MB_REG_ADDR_BAUDRATE_L,
	MB_REG_ADDR_ZB_ADDR_START,
	MB_REG_ADDR_ZB_ADDR_END = MB_REG_ADDR_ZB_ADDR_START + 11,
	MB_REG_ADDR_UID_BUF_START,
	MB_REG_ADDR_UID_BUF_END = MB_REG_ADDR_UID_BUF_START + 7,
	MB_REG_ADDR_RWA_MAX,

	MB_REG_ADDR_TEST_BASE = 65530,
	MB_REG_ADDR_TEST_1 = MB_REG_ADDR_TEST_BASE,
	MB_REG_ADDR_TEST_2,
	MB_REG_ADDR_TEST_MAX,
	MB_REG_ADDR_TH_RW_BASE = 10000,
    MB_REG_ADDR_WARN_MASK = MB_REG_ADDR_TH_RW_BASE,
    MB_REG_ADDR_TH_T_HIGH,
    MB_REG_ADDR_TH_T_LOW,
    MB_REG_ADDR_TH_RH_HIGH,
    MB_REG_ADDR_TH_RH_LOW,
    MB_REG_ADDR_RECORD_INTV,
    MB_REG_ADDR_TH_RW_MAX,
	MB_REG_ADDR_MAX
} mb_reg_addr_t;

typedef enum mb_reg_permission{
	MB_REG_PERM_R = 0,
	MB_REG_PERM_RW,
	MB_REG_PERM_RWA,
}mb_reg_perm_t;
typedef enum mb_reg_type{
	MB_REG_T_HR = 1,
	MB_REG_T_IR,
	MB_REG_T_COIL,
	MB_REG_T_DI,
}mb_reg_type_t;

typedef union mb_seg_type{
	mb_reg_type_t reg_type;
	mb_reg_perm_t access_perm;
} mb_seg_type_t;

typedef struct mb_reg_segment{
	mb_seg_type_t type;
	uint16_t addr_start;
	uint16_t len;
	uint16_t *content;
}mb_reg_segment_t;


void modbus_regs_init();
void modbus_reg_update(mb_reg_addr_t addr, uint16_t value);
uint16_t modbus_reg_get(mb_reg_addr_t addr);
void modbus_reg_update_uid(const uint8_t *uid, uint16_t len);
ModbusError modbus_reg_callback(void *ctx, 
	const ModbusRegisterCallbackArgs *args,
	ModbusRegisterCallbackResult *out);
uint16_t *modbus_reg_buf_addr(mb_reg_addr_t addr);

#endif
