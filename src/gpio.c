#include "CH58x_common.h"
#include "gpio.h"
#include "utils.h"
#include "app_config.h"

#define TAG "GPIO"

typedef struct gpio_intr_context{
	gpio_intr_handler handler;
	void *user_ctx;
}gpio_intr_ctx_t;

typedef struct gpio_context{
	uint16_t int_mask_a;
	uint16_t int_mask_b;
	gpio_intr_ctx_t intr_ctx[GPIO_NUM_MAX];
}gpio_ctx_t;

static gpio_ctx_t gpio_ctx;

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
	uint16_t flag = GPIOA_ReadITFlagPort() & gpio_ctx.int_mask_a;
	int i = 0;
	if (flag){
    	GPIOA_ClearITFlagBit(flag);
		for (i = 0; i < 16; i++){
			if (flag & 0x01){
				if (gpio_ctx.intr_ctx[i].handler){
					gpio_ctx.intr_ctx[i].handler(gpio_ctx.intr_ctx[i].user_ctx);
				}
			}
			flag >>= 1;
			if (!flag){
				break;
			}
		}
	}
}

__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void)
{
    uint16_t flag = GPIOB_ReadITFlagPort() & gpio_ctx.int_mask_b;
	int i = 0;
	int pin = GPIO_NUM_PB_0;
	if (flag){
    	GPIOB_ClearITFlagBit(flag);
		for (i = GPIO_NUM_PB_0; i < 16; i++, pin++){
			if (flag & 0x01){
				if (gpio_ctx.intr_ctx[pin].handler){
					gpio_ctx.intr_ctx[pin].handler(gpio_ctx.intr_ctx[pin].user_ctx);
				}
			}
			flag >>= 1;
			if (!flag){
				break;
			}
		}
	}
}


int gpio_set_direction(gpio_num_t num, gpio_mode_t mode){
	if (!GPIO_VALID(num)){
		LOG_ERROR(TAG, "invalid gpio num(%d)", num);
		goto fail;
	}
	if (GPIO_PA(num)){
		GPIOA_ModeCfg(BIT0 << (num - GPIO_NUM_PA_0), mode);
	}
	if (GPIO_PB(num)){
		GPIOB_ModeCfg(BIT0 << (num - GPIO_NUM_PB_0), mode);
	}
	return 0;
fail:
	return -1;
}

int gpio_set_level(gpio_num_t num, int level){
	if (!GPIO_VALID(num)){
		LOG_ERROR(TAG, "invalid gpio num(%d)", num);
		goto fail;
	}
	if (GPIO_PA(num)){
		if (level){
			GPIOA_SetBits(BIT0 << (num - GPIO_NUM_PA_0));
		}else{
			GPIOA_ResetBits(BIT0 << (num - GPIO_NUM_PA_0));
		}
	}
	if (GPIO_PB(num)){
		if (level){
			GPIOB_SetBits(BIT0 << (num - GPIO_NUM_PB_0));
		}else{
			GPIOB_ResetBits(BIT0 << (num - GPIO_NUM_PB_0));
		}
	}
	return 0;
fail:
	return -1;
}

int gpio_get_level(gpio_num_t num){
	uint32_t pin = 0;
	if (!GPIO_VALID(num)){
		LOG_ERROR(TAG, "invalid gpio num(%d)", num);
		goto fail;
	}
	if (GPIO_PA(num)){
		pin = BIT0 << (num - GPIO_NUM_PA_0);
		return GPIOA_ReadPortPin(pin) ? LEVEL_HIGH : LEVEL_LOW;
	}
	if (GPIO_PB(num)){
		pin = BIT0 << (num - GPIO_NUM_PB_0);
		return GPIOB_ReadPortPin(pin) ? LEVEL_HIGH : LEVEL_LOW;
	}
	return 0;
fail:
	return -1;
}

int gpio_intr_enable(gpio_num_t num, gpio_intr_mode_t mode, 
	gpio_intr_handler handler, void *ctx)
{
	uint32_t pin = 0;
	if (!GPIO_VALID(num)){
		LOG_ERROR(TAG, "invalid gpio num(%d)", num);
		goto fail;
	}
	if (GPIO_PA(num)){
		pin = BIT0 << (num - GPIO_NUM_PA_0);
		GPIOA_ITModeCfg(pin, mode);
		PFIC_EnableIRQ(GPIO_A_IRQn);
		gpio_ctx.intr_ctx[num].handler = handler;
		gpio_ctx.intr_ctx[num].user_ctx = ctx;
		gpio_ctx.int_mask_a = BITS_SET(gpio_ctx.int_mask_a, pin);
	}
	if (GPIO_PB(num)){
		pin = BIT0 << (num - GPIO_NUM_PB_0);
		GPIOB_ITModeCfg(pin, mode);
		PFIC_EnableIRQ(GPIO_B_IRQn);
		gpio_ctx.intr_ctx[num].handler = handler;
		gpio_ctx.intr_ctx[num].user_ctx = ctx;
		gpio_ctx.int_mask_b = BITS_SET(gpio_ctx.int_mask_b, pin);
	}
	return 0;
fail:
	return -1;
}

int gpio_intr_disable(gpio_num_t num){
	uint32_t pin = 0;
	if (!GPIO_VALID(num)){
		LOG_ERROR(TAG, "invalid gpio num(%d)", num);
		goto fail;
	}
	if (GPIO_PA(num)){
		pin = BIT0 << (num - GPIO_NUM_PA_0);
		R16_PA_INT_EN = BITS_CLEAR(R16_PA_INT_EN, pin);
		R16_PA_INT_IF = pin;
		gpio_ctx.intr_ctx[num].handler = NULL;
		gpio_ctx.intr_ctx[num].user_ctx = NULL;
		gpio_ctx.int_mask_a = BITS_CLEAR(gpio_ctx.int_mask_a, pin);
		if (!gpio_ctx.int_mask_a){
			PFIC_DisableIRQ(GPIO_A_IRQn);
		}
	}
	if (GPIO_PB(num)){
		pin = BIT0 << (num - GPIO_NUM_PB_0);
		R16_PB_INT_EN = BITS_CLEAR(R16_PB_INT_EN, pin);
		R16_PB_INT_IF = pin;
		gpio_ctx.intr_ctx[num].handler = NULL;
		gpio_ctx.intr_ctx[num].user_ctx = NULL;
		gpio_ctx.int_mask_b = BITS_CLEAR(gpio_ctx.int_mask_b, pin);
		if (!gpio_ctx.int_mask_b){
			PFIC_DisableIRQ(GPIO_B_IRQn);
		}
	}
	return 0;
fail:
	return -1;
}

