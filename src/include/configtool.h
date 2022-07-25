#ifndef __CONFIG_TOOL_H__
#define __CONFIG_TOOL_H__

#include "stdint.h"

typedef struct cfg_ota_s{
	uint32_t app_version;
	uint32_t app_size;
	uint8_t app_md5[16];
	uint32_t ota_version;
	uint32_t ota_size;
	uint8_t ota_md5[16];
}cfg_ota_t;

typedef enum cfg_key{
	CFG_KEY_MB_ADDR = 1,
	CFG_KEY_MB_UART,
	CFG_KEY_OTA,
	CFG_KEY_SN,
	CFG_KEY_MAX,
} cfg_key_t;

typedef enum cfg_uart_par{
	CFG_UART_PAR_BASE = 0,
	CFG_UART_PAR_NONE = CFG_UART_PAR_BASE,
	CFG_UART_PAR_ODD,
	CFG_UART_PAR_EVEN,
	CFG_UART_PAR_MARK,
	CFG_UART_PAR_SPACE,
	CFG_UART_PAR_MAX,
}cfg_uart_par_t;
#define CFG_UART_PAR_VALID(p) ((p) >= CFG_UART_PAR_BASE && (p) < CFG_UART_PAR_MAX)
typedef enum cfg_uart_sb{
	CFG_UART_SB_BASE = 0,
	CFG_UART_SB_1 = CFG_UART_SB_BASE,
	CFG_UART_SB_1_5,
	CFG_UART_SB_2,
	CFG_UART_SB_MAX,
}cfg_uart_sb_t;
#define CFG_UART_SB_VALID(sb) ((sb) >= CFG_UART_SB_BASE && (sb) < CFG_UART_SB_MAX)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
typedef struct cfg_uart{
	uint32_t baudrate;
	uint8_t databits;	//data bits, range:5 - 8
	uint8_t parity:4;
	uint8_t stopbits:4;
} cfg_uart_t;
#else
typedef struct cfg_uart{
	uint32_t baudrate;
	uint8_t databits;	//data bits, range:5 - 8
	uint8_t stopbits:4;
	uint8_t parity:4;
} cfg_uart_t;

#endif

#define CFG_SN_LEN	16

int cfg_init();
int cfg_get_ota(cfg_ota_t *result);
int cfg_update_ota(cfg_ota_t *val);
int cfg_get_mb_addr(uint8_t *result);
int cfg_update_mb_addr(uint8_t val);
int cfg_get_mb_uart(cfg_uart_t *result);
int cfg_update_mb_uart(cfg_uart_t *val);
int cfg_get_sn(char *out);
int cfg_update_sn(char *sn);

#endif
