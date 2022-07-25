#ifndef __CRC_H__ 
#define __CRC_H__ 

#include <stdint.h> 
#include <stdbool.h> 
typedef struct {
    uint8_t width;
    uint32_t poly; 
    uint32_t init; 
    bool refin; 
    bool refout; 
    uint32_t xor_out; 
} crc_type_t; 

#define CRC4_ITU_INIT {4, 0x03, 0x00, true, true, 0x00}
#define CRC5_EPC_INIT {5, 0x09, 0x09, false, false, 0x00}
#define CRC5_ITU_INIT {5, 0x15, 0x00, true, true, 0x00}
#define CRC5_USB_INIT {5, 0x05, 0x1f, true, true, 0x1f}
#define CRC6_ITU_INIT {6, 0x03, 0x00, true, true, 0x00}
#define CRC7_MMC_INIT {7, 0x09, 0x00, false, false, 0x00}
#define CRC8_INIT {8, 0x07, 0x00, false, false, 0x00}
#define CRC8_ITU_INIT {8, 0x07, 0x00, false, false, 0x55}
#define CRC8_ROHC_INIT {8, 0x07, 0xff, true, true, 0x00}
#define CRC8_MAXIM_INIT {8, 0x31, 0x00, true, true, 0x00}
#define CRC16_IBM_INIT {16, 0x8005, 0x0000, true, true, 0x0000}
#define CRC16_MAXIM_INIT {16, 0x8005, 0x0000, true, true, 0xffff}
#define CRC16_USB_INIT {16, 0x8005, 0xffff, true, true, 0xffff}
#define CRC16_MODBUS_INIT {16, 0x8005, 0xffff, true, true, 0x0000}
#define CRC16_CCITT_INIT {16, 0x1021, 0x0000, true, true, 0x0000}
#define CRC16_CCITT_FALSE_INIT {16, 0x1021, 0xffff, false, false, 0x0000}
#define CRC16_X25_INIT {16, 0x1021, 0xffff, true, true, 0xffff}
#define CRC16_XMODEM_INIT {16, 0x1021, 0x0000, false, false, 0x0000}
#define CRC16_DNP_INIT {16, 0x3D65, 0x0000, true, true, 0xffff}
#define CRC32_INIT {32, 0x04c11db7, 0xffffffff, true, true, 0xffffffff}
#define CRC32_MPEG2_INIT {32, 0x4c11db7, 0xffffffff, false, false, 0x00000000}

uint32_t crc_check(crc_type_t *crc_type, const uint8_t * buffer, uint32_t length); 

#endif