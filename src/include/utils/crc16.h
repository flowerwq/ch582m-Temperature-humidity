#ifndef __CRC16_H__
#define __CRC16_H__
#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
struct crc16_unit{
	uint8_t high;
	uint8_t low;
};
#else
struct crc16_unit{
	uint8_t low;
	uint8_t high;
};

#endif
union crc16_data{
	struct crc16_unit unit;
	uint16_t value;
};

typedef struct crc16_context{
	union crc16_data crcdata;
	uint16_t len;
}crc16_ctx_t;

//uint16_t CRC16(unsigned char *puchMsg, unsigned int usDataLen);
void crc16_init(crc16_ctx_t *ctx);
void crc16_update(crc16_ctx_t *ctx, uint8_t *buf, uint16_t len);
uint16_t crc16_value(crc16_ctx_t *ctx);
uint16_t crc16(uint8_t *buf, uint16_t len);

#endif
