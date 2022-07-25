#include "configtool.h"
#include "storage.h"
#include "utils.h"

#define TAG "CFG"

typedef int (*config_item_encode_func)(void *content, int len, uint8_t *pout);
typedef int (*config_item_decode_func)(void *item);

typedef struct cfg_cache_s{
	uint8_t mb_addr;
	cfg_uart_t mb_uart;
	cfg_ota_t ota;
	char sn[CFG_SN_LEN];
} cfg_cache_t;

static cfg_cache_t cfg_cache = {0};
static uint8_t cfg_flag_init = 0;
typedef struct cfg_item{
	uint16_t key;
	void *content;
	int size;
	config_item_encode_func encode_func;
	config_item_decode_func decode_func;
} cfg_item_t;

typedef enum cfg_idx{
	CFG_IDX_BASE = 0,
	CFG_IDX_MB_ADDR = CFG_IDX_BASE,
	CFG_IDX_MB_UART,
	CFG_IDX_OTA,
	CFG_IDX_SN,
	CFG_IDX_MAX,
} cfg_idx_t;
#define CFG_IDX_VALID(i)	((i) >= CFG_IDX_BASE && (i) < CFG_IDX_MAX)
static struct cfg_item cfg_items[] = {
	{CFG_KEY_MB_ADDR, &cfg_cache.mb_addr, sizeof(cfg_cache.mb_addr), NULL, NULL},
	{CFG_KEY_MB_UART, &cfg_cache.mb_uart, sizeof(cfg_cache.mb_uart), NULL, NULL},
	{CFG_KEY_OTA, &cfg_cache.ota, sizeof(cfg_cache.ota), NULL, NULL},
	{CFG_KEY_SN, &cfg_cache.sn, sizeof(cfg_cache.sn), NULL, NULL},
};

static int cfg_is_init(){
	return cfg_flag_init;
}

static int cfg_save_item(cfg_idx_t idx, void *content, int len){
	int ret = 0;
	//int buflen;
	if (!CFG_IDX_VALID(idx)){
		LOG_ERROR(TAG, "%s:invalid idx(%d)", __FUNCTION__, idx);
		return -1;
	}
	struct cfg_item *item = &cfg_items[idx];
	uint8_t buf[2 * item->size];
	
	if (item->encode_func){
		ret = item->encode_func(content, len, buf);
		if (ret < 0){
			LOG_ERROR(TAG, "fail to encode %04X", item->key);
			goto fail;
		}
	}else{
		memset(buf, 0, item->size);
		memcpy(buf, content, MIN(item->size, len));
	}
	ret = st_write_item(item->key, buf, item->size);
	if (ret < 0){
		LOG_ERROR(TAG, "fail to save %04X.", item->key);
		goto fail;
	}
	return 0;
fail:
	return -1;
}

int cfg_get_mb_uart(cfg_uart_t *result){
	//int ret = 0;
	if (!result){
		return -1;
	}
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	memcpy(result, &cfg_cache.mb_uart, sizeof(cfg_uart_t));
	return 0;
}

int cfg_update_mb_uart(cfg_uart_t *val){
	int ret = 0;
	if (!val){
		return -1;
	}
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	if (memcmp(&cfg_cache.mb_uart, val, sizeof(cfg_uart_t))){
		ret = cfg_save_item(CFG_IDX_MB_UART, val, sizeof(cfg_uart_t));
		if (ret < 0){
			return -1;
		}
		memcpy(&cfg_cache.mb_uart, val, sizeof(cfg_uart_t));
	}
	return 0;
}

int cfg_get_ota(cfg_ota_t *result){
	//int ret = 0;
	if (!result){
		return -1;
	}
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	memcpy(result, &cfg_cache.ota, sizeof(cfg_ota_t));
	return 0;
}

int cfg_update_ota(cfg_ota_t *val){
	int ret = 0;
	if (!val){
		return -1;
	}
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	if (memcmp(&cfg_cache.ota, val, sizeof(cfg_ota_t))){
		ret = cfg_save_item(CFG_IDX_OTA, val, sizeof(cfg_ota_t));
		if (ret < 0){
			return -1;
		}
		memcpy(&cfg_cache.ota, val, sizeof(cfg_ota_t));
	}
	return 0;
}
int cfg_get_mb_addr(uint8_t *result){
	//int ret = 0;
	if (!result){
		return -1;
	}
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	memcpy(result, &cfg_cache.mb_addr, sizeof(uint8_t));
	return 0;
}

int cfg_update_mb_addr(uint8_t val){
	int ret = 0;
	if (!val){
		return -1;
	}
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	if (memcmp(&cfg_cache.mb_addr, &val, sizeof(uint8_t))){
		ret = cfg_save_item(CFG_IDX_MB_ADDR, &val, sizeof(uint8_t));
		if (ret < 0){
			return -1;
		}
		memcpy(&cfg_cache.mb_addr, &val, sizeof(uint8_t));
	}
	return 0;
}

int cfg_get_sn(char *out){
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	if (out){
		memcpy(out, cfg_cache.sn, CFG_SN_LEN);
	}
	return 0;
}

int cfg_update_sn(char *sn){
	int ret = 0;
	if (!cfg_is_init()){
		LOG_ERROR(TAG, "%s:not init", __FUNCTION__);
		return -1;
	}
	if (strncmp(sn, (char *)cfg_cache.sn, CFG_SN_LEN)){
		ret = cfg_save_item(CFG_IDX_SN, sn, CFG_SN_LEN);
		if (ret < 0){
			return -1;
		}
		memcpy(cfg_cache.sn, sn, CFG_SN_LEN);
	}
	
	return 0;
}


static int cfg_load_default(){
	memset(&cfg_cache, 0, sizeof(cfg_cache));
	cfg_cache.mb_addr = 1;
	cfg_cache.mb_uart.baudrate = 9600;
	cfg_cache.mb_uart.databits = 8;
	cfg_cache.mb_uart.parity = CFG_UART_PAR_NONE;
	cfg_cache.mb_uart.stopbits = CFG_UART_SB_1;
	return 0;
}

static int cfg_item_init(cfg_item_t *item){
	if (!item){
		return -1;
	}
	int ret = st_read_item(item->key, item->content, item->size);
	if (ret <= 0){
		if (0 == ret){
			LOG_INFO(TAG, "%04X not found. write new one.", item->key);
			ret = st_write_item(item->key, item->content, item->size);
			if (ret < 0){
				LOG_ERROR(TAG, "fail to save %04X.", item->key);
				return -1;
			}
		}else{
			LOG_ERROR(TAG, "fail to get %04X", item->key);
			return -1;
		}
	}
	if (item->decode_func){
		item->decode_func(item);
	}
	return 0;
}

int cfg_init(){
	int ret = 0;
	int i = 0;
	if (cfg_flag_init){
		return 0;
	}
	ret = st_init();
	if (ret < 0){
		return -1;
	}
	cfg_load_default();
	for(i = 0; i < ARRAY_SIZE(cfg_items); i++){
		cfg_item_init(&cfg_items[i]);
	}
	cfg_flag_init = 1;
	return 0;
}

