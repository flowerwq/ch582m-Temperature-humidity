#include "stdio.h"
#include "CH58x_common.h"
#include "worktime.h"
#include "modbus.h"
#include "configtool.h"
#include "appinfo.h"
#include "utils.h"
#include "uid.h"
#include "sensor.h"
#include "oled.h"
#define TAG "SENSOR"

#define MB_OPT_RESET 0
#define MB_OPT_UPGRADE 	1

#define MB_OPT_RELOAD	3
typedef struct sensor_msg_th
{
    float th;
    float ph;
}MSG_TH;


typedef enum i2c_sensor_stat
{
    I2C_SENSOR_INIT,
    I2C_SENSOR_REST,
    I2C_SENSOR_SEND_CMD,
    I2C_SENSOR_JUDGE_STA,
    I2C_SENSOR_READ_VAL
}I2C_SENSOR_S;



static uint16_t invtFlag = 0;
static uint16_t ledFlag = 0;

static uint16_t timerCout = 0;
static uint16_t ledCout = 0;

static MSG_TH th_msg;
static UINT8 i2c_time_flag = APP_SENSOR_FALSE;
static I2C_SENSOR_S i2c_sensor_st = I2C_SENSOR_INIT;
static UINT8 i2c_time_cout = 0;
typedef  struct sensor_context {
	uint8_t flag_init:1;
	uint8_t flag_reset:1;
	uint64_t worktime;
}sensor_ctx_t;
static sensor_ctx_t sensor_ctx;

static bool baudrate_check(uint32_t baudrate){
	switch(baudrate){
		case 4800:
		case 9600:
		case 19200:
		case 38400:
		case 57600:
		case 115200:
		case 230400:
		case 380400:
		case 460800:
		case 921600:
			break;
		default:
			goto fail;
	}
	return true;
fail:
	return false;
}

static int sensor_reload_uart_config(){
	cfg_uart_t cfg_uart = {0};
	cfg_get_mb_uart(&cfg_uart);
	cfg_uart.baudrate = modbus_reg_get(MB_REG_ADDR_BAUDRATE_H);
	cfg_uart.baudrate <<= 16;
	cfg_uart.baudrate += modbus_reg_get(MB_REG_ADDR_BAUDRATE_L);
	if (!baudrate_check(cfg_uart.baudrate)){
		LOG_ERROR(TAG, "invalid baudrate(%lu)", cfg_uart.baudrate);
		goto fail;
	}
	LOG_DEBUG(TAG, "update uart config(baudrate:%lu)", cfg_uart.baudrate);
	cfg_update_mb_uart(&cfg_uart);
	return 0;
fail:
	return -1;
}

static int sensor_reload_mb_addr(){
	uint16_t tmp = 0;
	tmp = modbus_reg_get(MB_REG_ADDR_MB_ADDR);
	if (tmp < 1 || tmp > 247){
		LOG_ERROR(TAG, "invalid modbus address(%d)", tmp);
		modbus_reg_update(MB_REG_ADDR_MB_ADDR, tmp);
		goto fail;
	}
	LOG_DEBUG(TAG, "update mb_addr:%d", tmp);
	cfg_update_mb_addr((uint8_t)tmp);
	return 0;
fail:
	return -1;
}

static int sensor_reload_sn(){
	uint8_t sn[CFG_SN_LEN + 1] = {0};
	uint16_t *buf = modbus_reg_buf_addr(MB_REG_ADDR_UID_BUF_START);
	int bytes_remain = CFG_SN_LEN;
	int i = 0;
	if (!buf[0]){
		goto fail;
	}
	for(i = 0; i <= MB_REG_ADDR_UID_BUF_END - MB_REG_ADDR_UID_BUF_START; i ++){
		if (!buf[i]){
			break;
		}
		sn[i * 2] = buf[i] >> 8;
		if (sn[i * 2] > 127){
			LOG_ERROR(TAG, "invalid byte(0x%02X)", sn[i * 2]);
			goto fail;
		}
		bytes_remain --;
		if (!bytes_remain){
			break;
		}
		sn[i * 2 + 1] = buf[i] & 0xffU;
		if (sn[i * 2 + 1] > 127){
			LOG_ERROR(TAG, "invalid byte(0x%02X)", sn[i * 2 + 1]);
			goto fail;
		}
		bytes_remain --;
		if (!bytes_remain){
			break;
		}
	}
	sn[CFG_SN_LEN] = 0;
	if (sn[0]){
		LOG_DEBUG(TAG, "update sn: %s", sn);
		cfg_update_sn((char *)sn);
		modbus_reg_update_uid(sn,CFG_SN_LEN);
	}
	return 0;
fail:
	return -1;
}

/*
 * @brief Save configuration 
 */
static int sensor_reload_config(){
	sensor_reload_mb_addr();
	sensor_reload_uart_config();
	sensor_reload_sn();
	modbus_reload();
	return 0;
}

static int mb_addr_check(uint16_t value){
	return value >= 1 && value <= 247;
}

static int mb_reg_before_write(mb_reg_addr_t addr, uint16_t value){
	switch(addr){
		case MB_REG_ADDR_SAFE_ACCESS_CTRL:
			if (value && value != 0x3736 ){
				goto fail;
			}
			break;
		case MB_REG_ADDR_MB_ADDR:
			if (!mb_addr_check(value)){
				goto fail;
			}
			break;
		default:
			break;
	}
	return 0;
fail:
	return -1;
}

static int mb_opt_handle(uint16_t action){
	switch(action){
		case MB_OPT_RESET:
		case MB_OPT_UPGRADE:
			sensor_ctx.flag_reset = 1;
			break;
		case MB_OPT_RELOAD:
			sensor_reload_config();
			break;
		default:
			LOG_ERROR(TAG, "unknown operation(%d)", action);
			break;
	}
	return 0;
}

static int mb_reg_after_write(mb_reg_addr_t addr, uint16_t value){
	switch(addr){
		case MB_REG_ADDR_TEST_1:
			if (value != 0x5AA5){
				modbus_reg_update(MB_REG_ADDR_TEST_1, 0x5AA5);
			}
			break;
		case MB_REG_ADDR_TEST_2:
			modbus_reg_update(MB_REG_ADDR_TEST_2, ~value);
			break;
		case MB_REG_ADDR_OPT_CTRL:
			mb_opt_handle(value);
			break;
		case MB_REG_ADDR_SAFE_ACCESS_CTRL:
			if (0x3736 == value){
				modbus_sa_ctrl(1);
			}else if (0 == value){
				modbus_sa_ctrl(0);
			}
			break;
		default:
			break;
	}
	return 0;
}

int mb_init(){
	mb_callback_t mb_cbs  = {0};
	uint8_t mb_addr = 0;
	cfg_uart_t cfg_uart;
	uint8_t sn[CFG_SN_LEN] = {0};
	const appinfo_t *appinfo = appinfo_get();
	mb_cbs.before_reg_write = mb_reg_before_write;
	mb_cbs.after_reg_write = mb_reg_after_write;
	modbus_init(&mb_cbs);
	modbus_reg_update(MB_REG_ADDR_APP_STATE, MB_REG_APP_STATE_APP);
	modbus_reg_update(MB_REG_ADDR_APP_VID, appinfo->vid);
	modbus_reg_update(MB_REG_ADDR_APP_PID, appinfo->pid);
	modbus_reg_update(MB_REG_ADDR_VERSION_H, appinfo->version >> 16);
	modbus_reg_update(MB_REG_ADDR_VERSION_L, appinfo->version);
	modbus_reg_update(MB_REG_ADDR_HW_VERSION, appinfo->hw_version);
	modbus_reg_update(MB_REG_ADDR_TEST_1, 0x5AA5);
	cfg_get_mb_addr(&mb_addr);
	cfg_get_mb_uart(&cfg_uart);
	cfg_get_sn((char *)sn);
	modbus_reg_update_uid(sn, CFG_SN_LEN);
	modbus_reg_update(MB_REG_ADDR_MB_ADDR, mb_addr);
	modbus_reg_update(MB_REG_ADDR_BAUDRATE_H, cfg_uart.baudrate >> 16);
	modbus_reg_update(MB_REG_ADDR_BAUDRATE_L, cfg_uart.baudrate & 0xffffU);
	return 0;
}


int sensor_init(){
	if (sensor_ctx.flag_init){
		return 0;
	}
	memset(&sensor_ctx, 0, sizeof(sensor_ctx_t));
	mb_init();
	APP_sensor_init();
	sensor_ctx.flag_init = 1;
	return 0;
}


int sensor_run(){
	uint32_t worktime = 0;
	modbus_run();
	worktime = worktime_get()/1000;
	if (sensor_ctx.flag_reset){
		SYS_ResetExecute();
	}
	if (worktime != sensor_ctx.worktime){
		sensor_ctx.worktime = worktime;
		modbus_reg_update(MB_REG_ADDR_WORKTIME_H, (worktime >> 16) & 0xffffU);
		modbus_reg_update(MB_REG_ADDR_WORKTIME_L, worktime & 0xffffU);
	}
	return 0;
}

static void app_sensor_ledFlash(void)
{
    if (ledFlag == APP_SENSOR_TRUE)
    {
        GPIOB_InverseBits(GPIO_Pin_9);
        ledFlag = APP_SENSOR_FALSE;
    }

}

static void app_init_sysLed(void)
{
    GPIOB_SetBits(GPIO_Pin_9);
    GPIOB_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
}


__INTERRUPT
__HIGH_CODE
void TMR3_IRQHandler(void) // TMR3 定时中断
{
    UINT16 tmpTimerSum = 0;

    //PRINT("TMR3_IRQHandler\n");

    if(TMR3_GetITFlag(TMR0_3_IT_CYC_END)){
        TMR3_ClearITFlag(TMR0_3_IT_CYC_END);
    }
    //如果没有配置就以默认1S的时间读取
    if (0 == modbus_reg_get(MB_REG_ADDR_RECORD_INTV))
    {
        tmpTimerSum = 10;
    }
    else
    {
        tmpTimerSum = modbus_reg_get(MB_REG_ADDR_RECORD_INTV)*10;
    }

    timerCout++;
    if (timerCout > tmpTimerSum)
    {
        timerCout = 0;
        invtFlag = APP_SENSOR_TRUE;
    }

    ledCout++;
    if (ledCout > 10)
    {
        ledCout = 0;
        ledFlag = APP_SENSOR_TRUE;
    }
}



__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void) // TMR2 定时中断
{
    UINT16 tmpVal = 0;

    if(TMR2_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END);// 清除中断标志
    }

    if (i2c_time_flag == APP_SENSOR_FALSE)
    {
        ++i2c_time_cout;
        switch (i2c_sensor_st)
        {
            case I2C_SENSOR_INIT :
                tmpVal = 200;
                break;

            case I2C_SENSOR_REST :
                tmpVal = 200;
                break;

            case I2C_SENSOR_SEND_CMD :
                tmpVal = 20;
                break;

            case I2C_SENSOR_JUDGE_STA :
                tmpVal = 100;
                break;

            case I2C_SENSOR_READ_VAL :
                tmpVal = 10;
                break;

            default :
                break;
        }
        if (i2c_time_cout > tmpVal)
        {
            i2c_time_cout = 0;
            i2c_time_flag = APP_SENSOR_TRUE;
        }
    }
}



void app_sensor_modbus_dataFill(uint16_t *val)
{
    modbus_ireg_update(MB_IREG_ADDR_TH_T,val[0]);
    modbus_ireg_update(MB_IREG_ADDR_TH_H,val[1]);

}

static void  debug_msg(void )
{
    PRINT("app_sensor_dataDeal\r\n");
    //PRINT("%02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",o2_msg.start,o2_msg.cmd,o2_msg.o2Hight,o2_msg.o2low,o2_msg.name,o2_msg.point,o2_msg.res0,o2_msg.res1,o2_msg.check);
}

static void app_sensor_dataDeal(void)
{
    float    tmpVal;
    static uint16_t val[3] = {0};
    static char th_t[20]={0};
    static char th_p[20]={0};
//    static char sf[20]={0};
//    static char o2_t[20]={0};
    if (invtFlag == APP_SENSOR_TRUE)
    {
        //debug_msg();
//       memset(val,0,sizeof(val));
//  PRINT("name=:%2x\r\n",o2_msg.name);
       tmpVal = th_msg.th;
       val[0] = ((uint16_t)(tmpVal*10)) & 0xffff;
       tmpVal=th_msg.ph;
       val[1] = ((uint16_t)(tmpVal*10)) & 0xffff;

       sprintf(th_t,"%d.%d",val[0]/10,val[0]%10);
       OLED_ShowString(26, 16,(uint8_t *)th_t,16, 1);
       sprintf(th_p,"%d.%d",val[1]/10,val[1]%10);
       OLED_ShowString(26, 32,(uint8_t *)th_p,16, 1);

       app_sensor_modbus_dataFill(val);
    }
    invtFlag = APP_SENSOR_FALSE;
}

static int app_i2c_timerStart(uint32_t timeout_ms){
    i2c_time_flag = 0;
    TMR2_TimerInit(GetSysClock() / 1000  * timeout_ms);
    TMR3_Enable();
    return 0;
}

static void app_sensor_th_timerinit(void){
    TMR2_TimerInit(GetSysClock()/ 1000);         // 设置定时时间 1ms
    TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
    PFIC_EnableIRQ(TMR2_IRQn);
    TMR2_Enable();
}

void SDA_Pin_Output_High(void)   //将PB15配置为输出 ， 并设置为高电平， PB15作为I2C的SDA
{
    GPIOB_SetBits(GPIO_Pin_12);
    GPIOB_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_5mA);
}

void SDA_Pin_Output_Low(void)  //将P15配置为输出  并设置为低电平
{
    GPIOB_ResetBits(GPIO_Pin_12);
    GPIOB_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_5mA);
}

void SDA_Pin_IN_FLOATING(void)  //SDA配置为浮空输入
{
    GPIOB_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_Floating);
}

void SCL_Pin_Output_High(void) //SCL输出高电平，P14作为I2C的SCL
{
    GPIOB_SetBits(GPIO_Pin_13);
}

void SCL_Pin_Output_Low(void) //SCL输出低电平
{
    GPIOB_ResetBits(GPIO_Pin_13);
}

void I2C_SEND_Start(void)        //I2C主机发送START信号
{
    SDA_Pin_Output_High();
    DelayUs(8);
    SCL_Pin_Output_High();
    DelayUs(8);
    SDA_Pin_Output_Low();
    DelayUs(8);
    SCL_Pin_Output_Low();
    DelayUs(8);
}

void I2C_SEND_Stop(void)      //一条协议结束
{
    SDA_Pin_Output_Low();
    DelayUs(8);
    SCL_Pin_Output_High();
    DelayUs(8);
    SDA_Pin_Output_High();
    DelayUs(8);
}

void I2C_SEND_ACK(void)       //主机回复ACK信号
{
    SCL_Pin_Output_Low();
    DelayUs(8);
    SDA_Pin_Output_Low();
    DelayUs(8);
    SCL_Pin_Output_High();
    DelayUs(8);
    SCL_Pin_Output_Low();
    DelayUs(8);
    SDA_Pin_IN_FLOATING();
    DelayUs(8);
}

void I2C_WRITE_Byte(UINT8 Byte) //往AHT20写一个字节
{
    UINT8 Data,N,i;
    Data=Byte;
    i = 0x80;
    for(N=0;N<8;N++)
    {
        SCL_Pin_Output_Low();
        DelayUs(4);
        if(i&Data)
        {
            SDA_Pin_Output_High();
        }
        else
        {
            SDA_Pin_Output_Low();
        }

        SCL_Pin_Output_High();
        DelayUs(4);
        Data <<= 1;
    }
    SCL_Pin_Output_Low();
    DelayUs(8);
    SDA_Pin_IN_FLOATING();
    DelayUs(8);
}

UINT8 I2C_READ_Byte(void)
{
    UINT8 Byte,i,a;
    Byte = 0;
    SCL_Pin_Output_Low();
    SDA_Pin_IN_FLOATING();
    DelayUs(8);
    for(i=0;i<8;i++)
    {
        SCL_Pin_Output_High();
        DelayUs(5);
        a = 0;
        if(GPIOB_ReadPortPin(GPIO_Pin_12))
        {
            a = 1;

        }
        Byte = (Byte<<1)|a;
        //PRINT("I2C_READ_Byte %d :%d,%02x\r\n",i,a,Byte);
        SCL_Pin_Output_Low();
        DelayUs(5);
    }
    SDA_Pin_IN_FLOATING();
    DelayUs(8);
    return Byte;
}
UINT8 I2C_Receive_ACK(void)   //看AHT20是否有回复ACK
{
    UINT8 CNT = 0;
    SCL_Pin_Output_Low();
    SDA_Pin_IN_FLOATING();
    DelayUs(8);
    SCL_Pin_Output_High();
    DelayUs(8);
    while((GPIOB_ReadPortPin(GPIO_Pin_12))  && CNT < 100)
    CNT++;
    if(CNT == 100)
    {
        return 0;
    }
    SCL_Pin_Output_Low();
    DelayUs(8);
    return 1;
}

void I2C_Send_NOACK(void)   //主机不回复ACK
{
    SCL_Pin_Output_Low();
    DelayUs(8);
    SDA_Pin_Output_High();
    DelayUs(8);
    SCL_Pin_Output_High();
    DelayUs(8);
    SCL_Pin_Output_Low();
    DelayUs(8);
    SDA_Pin_Output_Low();
    DelayUs(8);
}
static void app_sensor_initAth21(UINT8 addr)
{
    UINT8 Byte_first,Byte_second,Byte_third,Byte_fourth;
    I2C_SEND_Start();
    I2C_WRITE_Byte(0x70);//原来是0x70
    I2C_Receive_ACK();
    I2C_WRITE_Byte(addr);
    I2C_Receive_ACK();
    I2C_WRITE_Byte(0x00);
    I2C_Receive_ACK();
    I2C_WRITE_Byte(0x00);
    I2C_Receive_ACK();
    I2C_SEND_Stop();

    DelayMs(5);//延时5ms左右
    I2C_SEND_Start();
    I2C_WRITE_Byte(0x71);//
    I2C_Receive_ACK();
    Byte_first = I2C_READ_Byte();
    I2C_SEND_ACK();
    Byte_second = I2C_READ_Byte();
    I2C_SEND_ACK();
    Byte_third = I2C_READ_Byte();
    I2C_Send_NOACK();
    I2C_SEND_Stop();

    DelayMs(10);//延时10ms左右
    I2C_SEND_Start();
    I2C_WRITE_Byte(0x70);///
    I2C_Receive_ACK();
    I2C_WRITE_Byte(0xB0|addr);////寄存器命令
    I2C_Receive_ACK();
    I2C_WRITE_Byte(Byte_second);
    I2C_Receive_ACK();
    I2C_WRITE_Byte(Byte_third);
    I2C_Receive_ACK();
    I2C_SEND_Stop();

    Byte_second=0x00;
    Byte_third =0x00;
    }

static void app_sensor_th_i2cWrite(UINT8 addr,UINT8 *data,UINT8 data_len)
{
    UINT8 i=0;

    I2C_SEND_Start();
    I2C_WRITE_Byte(addr << 1);
    I2C_Receive_ACK();
    while(i< data_len)
    {
        I2C_WRITE_Byte(data[i]);
        I2C_Receive_ACK();

        i = i + 1;
    }
    I2C_SEND_Stop();
}

static UINT8 app_sensor_th_i2cread(UINT8 addr,UINT8 *data,UINT8 data_len,UINT8 noAck)
{

    UINT8 i = 0;

    I2C_SEND_Start();
    I2C_WRITE_Byte((addr << 1) | 0x01);
    I2C_Receive_ACK();
    while(i< data_len)
    {
        data[i] = I2C_READ_Byte();
        //PRINT("app_sensor_th_i2cread data[%d]%02x \r\n",i,data[i]);
        i = i + 1;
        if (i >= data_len && noAck)
        {
            I2C_Send_NOACK();

        }
        else
        {
            I2C_SEND_ACK();

        }
    }

    I2C_SEND_Stop();
    return 0;
}

UINT8 app_sensor_CRC8(UINT8 *message,UINT8 Num)
{
    UINT8 i;
    UINT8 byte;
    UINT8 crc=0xFF;
    for(byte=0; byte<Num; byte++)
    {
        crc^=(message[byte]);
        for(i=8;i>0;--i)
        {
            if(crc&0x80) crc=(crc<<1)^0x31;
            else crc=(crc<<1);
        }
    }
    return crc;
}

static UINT8 app_sensor_th_staRead(UINT8 addr)
{

    UINT8 data = 0;

    I2C_SEND_Start();
    I2C_WRITE_Byte((addr << 1) | 0x01);
    I2C_Receive_ACK();
    data = I2C_READ_Byte();
    I2C_Send_NOACK();
    I2C_SEND_Stop();
    return data;
}

static void app_sensor_thRead(void)
{

    UINT8 buf[16];
    UINT8 crc = 0;
    INT32 tmpVal = 0;
    INT32 phVal  = 0;

    //PRINT("app_sensor_thRead\r\n\r\n\r\n\r\n");
    if (i2c_time_flag ==  APP_SENSOR_TRUE)
    {
        switch (i2c_sensor_st)
        {
            case I2C_SENSOR_INIT :
                app_sensor_th_i2cread(0x38,buf,1,1);
                if ((buf[0] & 0x18) != 0x18 )
                {
                    app_sensor_initAth21(0x1B);
                    app_sensor_initAth21(0x1C);
                    app_sensor_initAth21(0x1E);
                    i2c_sensor_st = I2C_SENSOR_REST;
                }
                else
                {
                    i2c_sensor_st = I2C_SENSOR_SEND_CMD;
                }
                break;

            case I2C_SENSOR_REST :
                i2c_sensor_st = I2C_SENSOR_INIT;
                break;

            case I2C_SENSOR_SEND_CMD :
                buf[0] = 0xAC;
                buf[1] = 0x33;
                buf[2] = 0x00;
                app_sensor_th_i2cWrite(0x38,buf,3);
                i2c_sensor_st = I2C_SENSOR_JUDGE_STA;
                break;

            case I2C_SENSOR_JUDGE_STA :
                app_sensor_th_i2cread(0x38,buf,1,1);
                if (buf[0] & 0x80 )
                {
                    i2c_sensor_st = I2C_SENSOR_JUDGE_STA;
                }
                else
                {
                    i2c_sensor_st = I2C_SENSOR_READ_VAL;
                }
                break;

            case I2C_SENSOR_READ_VAL :
                memset(buf,0,sizeof(buf));
                app_sensor_th_i2cread(0x38,buf,7,1);
#if 1
                crc = app_sensor_CRC8(buf,6);
                if (crc != buf[6])
                {
                    return;
                }
#endif
                phVal  =(INT32) buf[1] << 8;
                phVal = (phVal | buf[2]) <<8;
                phVal = (phVal | buf[3]) ;
                phVal = (phVal >> 4);
                th_msg.ph = (phVal *100) / 1048576.0;

                tmpVal = (INT32)buf[3] << 8;
                tmpVal = (tmpVal | buf[4]) <<8;
                tmpVal = (tmpVal | buf[5]) ;
                tmpVal = tmpVal & 0xfffff;
                th_msg.th = (tmpVal * 200) / 1048576.0 - 50;
                PRINT("th=:%.1f\r\n",th_msg.th );

                i2c_sensor_st = I2C_SENSOR_SEND_CMD;
                break;

            default :
                break;
        }
        i2c_time_flag = APP_SENSOR_FALSE;
    }
}


static void app_init_thSensor(void)
{
    memset(&th_msg,0,sizeof(th_msg));

    GPIOB_ResetBits(GPIO_Pin_12);
    GPIOB_ModeCfg(GPIO_Pin_12, GPIO_ModeOut_PP_5mA);
    GPIOB_ResetBits(GPIO_Pin_13);
    GPIOB_ModeCfg(GPIO_Pin_13, GPIO_ModeOut_PP_5mA);

    app_sensor_th_timerinit();
}

void app_sensor_timerInit(void)
{
    TMR3_TimerInit(GetSysClock()/ 10);         // 设置定时时间 100ms
    TMR3_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
    PFIC_EnableIRQ(TMR3_IRQn);
    TMR3_Enable();
}

void APP_sensor_init(void)
{
    PRINT("APP_sensor_init\n");
    app_init_thSensor();

    app_init_sysLed();
    app_sensor_timerInit();
}

void App_sensor_run(void)
{
    uint32_t worktime = 0;
    app_sensor_ledFlash();
    app_sensor_thRead();
#if 1
    app_sensor_dataDeal();

    modbus_run();
    worktime = worktime_get()/1000;
    if (worktime != sensor_ctx.worktime){
        sensor_ctx.worktime = worktime;
        modbus_reg_update(MB_REG_ADDR_WORKTIME_H, (worktime >> 16));
        modbus_reg_update(MB_REG_ADDR_WORKTIME_L, (worktime));
    }
    if (modbus_is_receiving()){
        return;
    }
#endif
}
