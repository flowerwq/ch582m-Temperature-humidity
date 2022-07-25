#include "CH58x_common.h"
#include "worktime.h"

static worktime_t worktime = 0;

__INTERRUPT
__HIGH_CODE
void SysTick_Handler(){
	worktime ++;
	SysTick->SR = 0;
}


int worktime_init(){
	SysTick_Config(GetSysClock()/1000);
	return 0;
}

/**
 * @brief get work time (in milliseconds) since boot
 * @return The number of milliseconds since boot
 */
worktime_t worktime_get(){
	return worktime;
}

/**
 * @brief Calculate work time interval
 * @param reference work time
 * @return The number of milliseconds since "from"
 */
worktime_t worktime_since(worktime_t from){
	return worktime - from;
}

