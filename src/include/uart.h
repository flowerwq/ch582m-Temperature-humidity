#ifndef __UART_H__
#define __UART_H__

#include "stdint.h"
#include "gpio.h"

typedef enum uart_num{
	UART_NUM_0 = 0,
	UART_NUM_1,
	UART_NUM_2,
	UART_NUM_3,
	UART_NUM_MAX,
} uart_num_t;

#define UART_NUM_VALID(n)	((n) >= UART_NUM_0 && (n) < UART_NUM_MAX)

typedef enum uart_par{
	UART_PAR_NONE = 0,
	UART_PAR_ODD,
	UART_PAR_EVEN,
	UART_PAR_MARK,
	UART_PAR_SPACE,
	UART_PAR_MAX,
} uart_par_t;

typedef enum uart_stopbits{
	UART_SB_1 = 0,
	UART_SB_1_5,	//NOT support by CH582M
	UART_SB_2,
	UART_SB_MAX,
}uart_sb_t;

typedef int (*uart_data_cb)(uart_num_t num, uint8_t *buf, uint32_t len, void *ctx);

typedef struct uart_config_s{
	uint8_t remap;	//!< remap the gpio
	uint32_t baudrate;	//!< uart baudrate
	uint8_t databits;	//!< uart data bits, accept value:5,6,7,8
	uart_sb_t stopbits;	//!< uart stop bits, only support 1 or 2 bits
	uart_par_t parity;	//!< UART parity, only support NONE or EVEN
	uint8_t rs485_en;	//!< enable rs485 mode rx/tx control
	gpio_num_t io_485en;	//!< io number for rs485 rx/tx control
	int level_485tx;	//!< active level for rs485 tx mode
	/**
	 * receive callback, leave it to NULL if you want receive data manully.
	 */
	uart_data_cb recv_cb;
	void *user_ctx;	//!< user context, will pass to user via recv_cb
}uart_config_t;

#define UART_DEFAULT_CONFIG()	{\
	.baudrate = 115200,\
	.databits = 8,\
	.stopbits = UART_SB_1,\
	.parity = UART_PAR_NONE,\
	}

int uart_init(uart_num_t num, uart_config_t *config);
int uart_send(uart_num_t num, uint8_t *buf, uint32_t len);
int uart_get_config(uart_num_t num, uart_config_t *config);
int uart_read(uart_num_t num, uint8_t *buf, uint16_t len, uint32_t timeout_ms);
int uart_rx_flush(uart_num_t num);
int uart_set_break(uart_num_t num, uint32_t time_ms);
int uart_deinit(uart_num_t num);

#endif
