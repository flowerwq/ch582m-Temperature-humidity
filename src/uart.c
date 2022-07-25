#include "CH58x_common.h"
#include "uart.h"
#include "utils.h"
#include "app_config.h"
#include "worktime.h"

#define TAG "UART"

#define UART_CACHE_MAX	1024

typedef struct uart_hal{
	PUINT8V mcr;	//modem control
	PUINT8V ier;	//interrupt enable
	PUINT8V fcr;	//FIFO control
	PUINT8V lcr;	//line control
	PUINT8V iir;	//interrupt identification
	PUINT8V lsr;	//line status
	PUINT8V msr;	//modem status
	PUINT8V rbr;	//receiver buffer, receiving byte
	PUINT8V thr;	//transmitter holding, transmittal byte
	PUINT8V rfc;	//receiver FIFO count
	PUINT8V tfc;
	PUINT16V dl;
	PUINT8V div;
	PUINT8V adr;
	PUINT16V pin_alternate;
}uart_hal_t;

typedef struct uart_context{
	uart_config_t config;
	gpio_num_t io_rx;
	gpio_num_t io_tx;
	gpio_num_t io_rx_remap;
	gpio_num_t io_tx_remap;
	uart_hal_t regs;
	int irq_num;
	uint8_t flag_init;
	uint16_t pin_alter_mask;
	uint8_t cache_buf[UART_CACHE_MAX];
	ringbuffer_t cache_info;
} uart_ctx_t;

static uart_ctx_t uart_ctx[UART_NUM_MAX] = {
	{
		.io_rx = GPIO_NUM_PB_4,
		.io_tx = GPIO_NUM_PB_7,
		.io_rx_remap = GPIO_NUM_PA_15,
		.io_tx_remap = GPIO_NUM_PA_14,
		.irq_num = UART0_IRQn,
		.pin_alter_mask = RB_PIN_UART0,
		.regs = {
			.mcr = &R8_UART0_MCR,
			.ier = &R8_UART0_IER,
			.fcr = &R8_UART0_FCR,
			.lcr = &R8_UART0_LCR,
			.iir = &R8_UART0_IIR,
			.lsr = &R8_UART0_LSR,
			.msr = &R8_UART0_MSR,
			.rbr = &R8_UART0_RBR,
			.thr = &R8_UART0_THR,
			.rfc = &R8_UART0_RFC,
			.tfc = &R8_UART0_TFC,
			.dl = &R16_UART0_DL,
			.div = &R8_UART0_DIV,
			.adr = &R8_UART0_ADR,
			.pin_alternate = &R16_PIN_ALTERNATE,
		}
	},
	{
		.io_rx = GPIO_NUM_PA_8,
		.io_tx = GPIO_NUM_PA_9,
		.io_rx_remap = GPIO_NUM_PB_12,
		.io_tx_remap = GPIO_NUM_PB_13,
		.irq_num = UART1_IRQn,
		.pin_alter_mask = RB_PIN_UART1,
		.regs = {
			.mcr = &R8_UART1_MCR,
			.ier = &R8_UART1_IER,
			.fcr = &R8_UART1_FCR,
			.lcr = &R8_UART1_LCR,
			.iir = &R8_UART1_IIR,
			.lsr = &R8_UART1_LSR,
			.rbr = &R8_UART1_RBR,
			.thr = &R8_UART1_THR,
			.rfc = &R8_UART1_RFC,
			.tfc = &R8_UART1_TFC,
			.dl = &R16_UART1_DL,
			.div = &R8_UART1_DIV,
			.pin_alternate = &R16_PIN_ALTERNATE,
		}
	},
	{
		.io_rx = GPIO_NUM_PA_6,
		.io_tx = GPIO_NUM_PA_7,
		.io_rx_remap = GPIO_NUM_PB_22,
		.io_tx_remap = GPIO_NUM_PB_23,
		.irq_num = UART2_IRQn,
		.pin_alter_mask = RB_PIN_UART2,
		.regs = {
			.mcr = &R8_UART2_MCR,
			.ier = &R8_UART2_IER,
			.fcr = &R8_UART2_FCR,
			.lcr = &R8_UART2_LCR,
			.iir = &R8_UART2_IIR,
			.lsr = &R8_UART2_LSR,
			.rbr = &R8_UART2_RBR,
			.thr = &R8_UART2_THR,
			.rfc = &R8_UART2_RFC,
			.tfc = &R8_UART2_TFC,
			.dl = &R16_UART2_DL,
			.div = &R8_UART2_DIV,
			.pin_alternate = &R16_PIN_ALTERNATE,
		}
	},
	{
		.io_rx = GPIO_NUM_PA_4,
		.io_tx = GPIO_NUM_PA_5,
		.io_rx_remap = GPIO_NUM_PB_20,
		.io_tx_remap = GPIO_NUM_PB_21,
		.irq_num = UART3_IRQn,
		.pin_alter_mask = RB_PIN_UART3,
		.regs = {
			.mcr = &R8_UART3_MCR,
			.ier = &R8_UART3_IER,
			.fcr = &R8_UART3_FCR,
			.lcr = &R8_UART3_LCR,
			.iir = &R8_UART3_IIR,
			.lsr = &R8_UART3_LSR,
			.rbr = &R8_UART3_RBR,
			.thr = &R8_UART3_THR,
			.rfc = &R8_UART3_RFC,
			.tfc = &R8_UART3_TFC,
			.dl = &R16_UART3_DL,
			.div = &R8_UART3_DIV,
			.pin_alternate = &R16_PIN_ALTERNATE,
		}
	},
};

static int uart_cache_write(uart_num_t num, uint8_t *buf, uint16_t len){
	uart_ctx_t *ctx = &uart_ctx[num];
	ringbuffer_write(&ctx->cache_info, buf, len);
	return 0;
}
static int uart_cache_read(uart_num_t num, uint8_t *buf, uint16_t len){
	uart_ctx_t *ctx = &uart_ctx[num];
	return ringbuffer_read(&ctx->cache_info, buf, len);
}

static int uart_cache_clear(uart_num_t num){
	uart_ctx_t *ctx = &uart_ctx[num];
	ringbuffer_drop(&ctx->cache_info, ringbuffer_get_length(&ctx->cache_info));
	return 0;
}

/*********************************************************************
 * @fn      UART0_IRQHandler
 *
 * @brief   UART0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    uint8_t d = 0;
	uart_ctx_t *ctx = &uart_ctx[UART_NUM_0];
    switch(UART0_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART0_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
            while(R8_UART0_RFC)
            {	
            	d = UART0_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_0, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_0, &d, 1);
				}
            }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            while(R8_UART0_RFC)
            {
                d = UART0_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_0, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_0, &d, 1);
				}
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    uint8_t d = 0;
	uart_ctx_t *ctx = &uart_ctx[UART_NUM_1];
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART1_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
            while(R8_UART1_RFC)
            {	
            	d = UART1_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_1, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_1, &d, 1);
				}
            }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            while(R8_UART1_RFC)
            {
                d = UART1_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_1, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_1, &d, 1);
				}
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART2_IRQHandler
 *
 * @brief   UART2中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART2_IRQHandler(void)
{
    uint8_t d = 0;
	uart_ctx_t *ctx = &uart_ctx[UART_NUM_2];
    switch(UART2_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART2_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
            while(R8_UART2_RFC)
            {	
            	d = UART2_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_2, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_2, &d, 1);
				}
            }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            while(R8_UART2_RFC)
            {
                d = UART2_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_2, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_2, &d, 1);
				}
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART3_IRQHandler
 *
 * @brief   UART3中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART3_IRQHandler(void)
{
    uint8_t d = 0;
	uart_ctx_t *ctx = &uart_ctx[UART_NUM_3];
    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART3_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点
            while(R8_UART3_RFC)
            {	
            	d = UART3_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_3, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_3, &d, 1);
				}
            }
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            while(R8_UART3_RFC)
            {
                d = UART3_RecvByte();
				if(ctx->config.recv_cb){
					ctx->config.recv_cb(UART_NUM_3, &d, 1, ctx->config.user_ctx);
				}else{
					uart_cache_write(UART_NUM_3, &d, 1);
				}
            }
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

static int uart_io_init(uart_num_t num, uart_config_t * config){
	uart_ctx_t *ctx = &uart_ctx[num];
	if (config->remap){
		gpio_set_direction(ctx->io_rx_remap, GPIO_INPUT_PU);
		gpio_set_direction(ctx->io_tx_remap, GPIO_OUTPUT_PP_20mA);
		gpio_set_level(ctx->io_tx_remap, LEVEL_HIGH);
		*ctx->regs.pin_alternate = BITS_SET(*ctx->regs.pin_alternate, 
			ctx->pin_alter_mask);
	}else{
		gpio_set_direction(ctx->io_rx, GPIO_INPUT_PU);
		gpio_set_direction(ctx->io_tx, GPIO_OUTPUT_PP_20mA);
		gpio_set_level(ctx->io_tx, LEVEL_HIGH);
		*ctx->regs.pin_alternate = BITS_CLEAR(*ctx->regs.pin_alternate, 
			ctx->pin_alter_mask);
	}
	if (config->rs485_en){
		gpio_set_direction(config->io_485en, GPIO_OUTPUT_PP_20mA);
		gpio_set_level(config->io_485en, !config->level_485tx);
	}
	return 0;
}
int uart_set_stopbits(uart_num_t num, uart_sb_t sb){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	switch(sb){
		case UART_SB_1:
			*ctx->regs.lcr = BITS_CLEAR(*ctx->regs.lcr, RB_LCR_STOP_BIT);
			break;
		case UART_SB_2:
			*ctx->regs.lcr = BITS_SET(*ctx->regs.lcr, RB_LCR_STOP_BIT);
			break;
		default:
			LOG_ERROR(TAG, "unsupported stopbits");
			goto fail;
			break;
	}
	return 0;
fail:
	return -1;
}
int uart_set_databits(uart_num_t num, uint8_t databits){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	uint8_t reg_val = *ctx->regs.lcr;
	reg_val = BITS_CLEAR(reg_val, RB_LCR_WORD_SZ);
	switch(databits){
		case 5:
			break;
		case 6:
			reg_val = BIT_SET(reg_val, 0);
			break;
		case 7:
			reg_val = BIT_SET(reg_val, 1);
			break;
		case 8:
			reg_val = BITS_SET(reg_val, RB_LCR_WORD_SZ);
			break;
		default:
			LOG_ERROR(TAG, "invalid databits(%d)", databits);
			goto fail;
			break;
	}
	*ctx->regs.lcr = reg_val;
	return 0;
fail:
	return -1;
}
int uart_set_baudrate(uart_num_t num, uint32_t baudrate){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	uint32_t x;

    x = 10 * GetSysClock() / 8 / baudrate;
    x = (x + 5) / 10;
    *ctx->regs.dl = (uint16_t)x;
	return 0;
fail:
	return -1;
}

int uart_set_parity(uart_num_t num, uart_par_t parity){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	uint8_t reg_val = *ctx->regs.lcr;
	switch(parity){
		case UART_PAR_NONE:
			reg_val = BITS_CLEAR(reg_val, RB_LCR_PAR_EN);
			break;
		case UART_PAR_EVEN:
			reg_val = BITS_SET(reg_val, RB_LCR_PAR_EN);
			break;
		default:
			LOG_ERROR(TAG, "unsupport parity");
			goto fail;
	}
	*ctx->regs.lcr = reg_val;
	return 0;
fail:
	return -1;
}
static void uart_int_config(uart_num_t num, int en, uint8_t mask)
{
	uart_ctx_t *ctx = &uart_ctx[num];
    if(en)
    {
        *ctx->regs.ier |= mask;
        *ctx->regs.mcr |= RB_MCR_INT_OE;
    }
    else
    {
        *ctx->regs.ier &= ~mask;
    }
}
static void uart_byte_trig_config(uart_num_t num, UARTByteTRIGTypeDef b)
{
	uart_ctx_t *ctx = &uart_ctx[num];
    *ctx->regs.fcr = (*ctx->regs.fcr & ~RB_FCR_FIFO_TRIG) | (b << 6);
}

static int uart_init_internal(uart_num_t num, uart_config_t *config){
	uart_ctx_t *ctx = &uart_ctx[num];
	uart_io_init(num, config);
	uart_set_baudrate(num, config->baudrate);
	uart_set_databits(num, config->databits);
	uart_set_stopbits(num, config->stopbits);
	uart_set_parity(num, config->parity);
	
	// FIFO打开，触发点1字节
	*ctx->regs.fcr = RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN;
	//tx en
    *ctx->regs.ier = RB_IER_TXD_EN;
    *ctx->regs.div = 1;
	
	ringbuffer_init(&ctx->cache_info, ctx->cache_buf, UART_CACHE_MAX);
	uart_byte_trig_config(num, UART_1BYTE_TRIG);
	uart_int_config(num, 1, RB_IER_LINE_STAT|RB_IER_RECV_RDY);
	PFIC_EnableIRQ(ctx->irq_num);
	return 0;
}

/**
 * @brief Initialize the UART
 * @param num UART number
 * @param config UART configuration
 * @return 0-Success, (-1)-Failure
 */
int uart_init(uart_num_t num, uart_config_t * config){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	if (!config){
		LOG_ERROR(TAG, "param err");
		goto fail;
	}
	int ret = uart_init_internal(num, config);
	if (ret < 0){
		LOG_ERROR(TAG, "uart%d init failed", num);
		goto fail;
	}
	memcpy(&uart_ctx[num].config, config, sizeof(uart_config_t));
	uart_ctx[num].flag_init = 1;
	return 0;
fail:
	return -1;
}

/**
 * @brief deinitialize uart
 * @param num uart number
 * @return 0 - Success, (-1) - Error
 */
int uart_deinit(uart_num_t num){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	PFIC_DisableIRQ(ctx->irq_num);
	*ctx->regs.ier = BITS_SET(*ctx->regs.ier, RB_IER_RESET);
	memset(&ctx->config, 0, sizeof(uart_config_t));
	ctx->flag_init = 0;
	return 0;
fail:
	return -1;
}

/**
 * @brief send data to uart.
 * @param num uart number
 * @param buf data buffer
 * @param len data length
 * @return 0 - Success, (-1) - failed.
 * @note This function will not work if you init uart with "recv_cb" set
 */
int uart_send(uart_num_t num, uint8_t * buf, uint32_t len){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	if (!buf || len <= 0){
		return 0;
	}
	if (ctx->config.rs485_en){
		gpio_set_level(ctx->config.io_485en, ctx->config.level_485tx);
	}
	while(len)
    {
        if(*ctx->regs.tfc != UART_FIFO_SIZE)
        {
            *ctx->regs.thr = *buf++;
            len--;
        }
    }
	while(*ctx->regs.tfc || !(*ctx->regs.lsr & RB_LSR_TX_ALL_EMP));
	if (ctx->config.rs485_en){
		gpio_set_level(ctx->config.io_485en, !ctx->config.level_485tx);
	}
	return 0;
fail:
	return -1;
}
/**
 * @brief read data from uart manually.
 * @param num uart number
 * @param buf data buffer
 * @param len buffer length
 * @param timeout_ms The maximum amount of time wait for data
 * @return the actual length(in bytes) of data received sucessfully.
 * @note This function will not work if you init uart with "recv_cb" set
 */
int uart_read(uart_num_t num, uint8_t *buf, uint16_t len, uint32_t timeout_ms){
	uint16_t byts_read = 0;
	worktime_t worktime = worktime_get();
	int ret = 0;
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	if (!ctx->flag_init){
		LOG_ERROR(TAG, "not init");
		goto fail;
	}
	while(worktime_since(worktime) < timeout_ms){
		ret = uart_cache_read(num, buf + byts_read, len - byts_read);
		if (ret < 0){
			LOG_ERROR(TAG, "cache read err");
			goto out;
		}
		byts_read += ret;
		if (byts_read >= len){
			goto out;
		}
	}
out:
	return byts_read;
fail:
	return -1;
}

int uart_get_config(uart_num_t num, uart_config_t *config){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	if (!config){
		LOG_ERROR(TAG, "param err");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	memcpy(config, &ctx->config, sizeof(uart_config_t));
	return 0;
fail:
	return -1;
}
int uart_rx_flush(uart_num_t num){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	uart_ctx_t *ctx = &uart_ctx[num];
	if (!ctx->flag_init){
		LOG_ERROR(TAG, "not init");
		goto fail;
	}
	PFIC_DisableIRQ(ctx->irq_num);
	
	*ctx->regs.fcr = BITS_SET(*ctx->regs.fcr, RB_FCR_RX_FIFO_CLR);
	uart_cache_clear(num);
	
	PFIC_EnableIRQ(ctx->irq_num);
	return 0;
fail:
	return -1;
}

int uart_set_break(uart_num_t num, uint32_t time_ms){
	if (!UART_NUM_VALID(num)){
		LOG_ERROR(TAG, "invalid uart num");
		goto fail;
	}
	worktime_t worktime = worktime_get();
	uart_ctx_t *ctx = &uart_ctx[num];
	if (!ctx->flag_init){
		LOG_ERROR(TAG, "not init");
		goto fail;
	}
	*ctx->regs.lcr = BITS_SET(*ctx->regs.lcr, RB_LCR_BREAK_EN);
	while(worktime_since(worktime) < time_ms){
		__nop();
	}
	*ctx->regs.lcr = BITS_CLEAR(*ctx->regs.lcr, RB_LCR_BREAK_EN);
	return 0;
fail:
	return -1;
}
