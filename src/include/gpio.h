#ifndef __GPIO_H__
#define __GPIO_H__

typedef enum gpio_num{
	GPIO_NUM_PA_0,
	GPIO_NUM_PA_1,
	GPIO_NUM_PA_2,
	GPIO_NUM_PA_3,
	GPIO_NUM_PA_4,
	GPIO_NUM_PA_5,
	GPIO_NUM_PA_6,
	GPIO_NUM_PA_7,
	GPIO_NUM_PA_8,
	GPIO_NUM_PA_9,
	GPIO_NUM_PA_10,
	GPIO_NUM_PA_11,
	GPIO_NUM_PA_12,
	GPIO_NUM_PA_13,
	GPIO_NUM_PA_14,
	GPIO_NUM_PA_15,
	GPIO_NUM_PA_MAX,
	GPIO_NUM_PB_0 = GPIO_NUM_PA_MAX,
	GPIO_NUM_PB_1,
	GPIO_NUM_PB_2,
	GPIO_NUM_PB_3,
	GPIO_NUM_PB_4,
	GPIO_NUM_PB_5,
	GPIO_NUM_PB_6,
	GPIO_NUM_PB_7,
	GPIO_NUM_PB_8,
	GPIO_NUM_PB_9,
	GPIO_NUM_PB_10,
	GPIO_NUM_PB_11,
	GPIO_NUM_PB_12,
	GPIO_NUM_PB_13,
	GPIO_NUM_PB_14,
	GPIO_NUM_PB_15,
	GPIO_NUM_PB_16,
	GPIO_NUM_PB_17,
	GPIO_NUM_PB_18,
	GPIO_NUM_PB_19,
	GPIO_NUM_PB_20,
	GPIO_NUM_PB_21,
	GPIO_NUM_PB_22,
	GPIO_NUM_PB_23,
	GPIO_NUM_PB_MAX,
	GPIO_NUM_MAX = GPIO_NUM_PB_MAX,
}gpio_num_t;

#define GPIO_PA(n)	((n) >= GPIO_NUM_PA_0 && (n) < GPIO_NUM_PA_MAX)
#define GPIO_PB(n)	((n) >= GPIO_NUM_PB_0 && (n) < GPIO_NUM_PB_MAX)
#define GPIO_VALID(n)	((n) >= GPIO_NUM_PA_0 && (n) < GPIO_NUM_MAX)

typedef enum gpio_mode{
	GPIO_INPUT_FLOATING, //浮空输入
    GPIO_INPUT_PU,       //上拉输入
    GPIO_INPUT_PD,       //下拉输入
    GPIO_OUTPUT_PP_5mA,  //推挽输出最大5mA
    GPIO_OUTPUT_PP_20mA, //推挽输出最大20mA
} gpio_mode_t;

typedef GPIOITModeTpDef gpio_intr_mode_t;

typedef int (*gpio_intr_handler)(void *ctx);

int gpio_set_direction(gpio_num_t num, gpio_mode_t mode);
int gpio_set_level(gpio_num_t num, int level);
int gpio_get_level(gpio_num_t num);
int gpio_intr_enable(gpio_num_t num, gpio_intr_mode_t mode, 
	gpio_intr_handler handler, void *ctx);
int gpio_intr_disable(gpio_num_t num);


#endif
