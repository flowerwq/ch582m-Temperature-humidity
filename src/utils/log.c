#include "stdio.h"
#include "string.h"
#include <ctype.h>
#include "CH58x_common.h"
#include "utils.h"
#define BYTES_PER_LINE 16

void log_buffer_hex(const char *tag, const void *buffer, uint16_t buff_len,
	log_level_t log_level)
{
    if (buff_len == 0) {
        return;
    }
    char hex_buffer[3 * BYTES_PER_LINE + 1];
    const unsigned char *ptr_line;
    int bytes_cur_line;

    do {
        if (buff_len > BYTES_PER_LINE) {
            bytes_cur_line = BYTES_PER_LINE;
        } else {
            bytes_cur_line = buff_len;
        }
        
        ptr_line = buffer;
        

        for (int i = 0; i < bytes_cur_line; i ++) {
            sprintf(hex_buffer + 3 * i, "%02x ", ptr_line[i]);
        }
        LOG_LEVEL(log_level, tag, "%s", hex_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}


void log_buffer_hexdump(const char *tag, const void *buffer, 
	uint16_t buff_len, log_level_t log_level)
{

    if (buff_len == 0) {
        return;
    }
    const unsigned char *ptr_line;
    //format: field[length]
    // ADDR[10]+"   "+DATA_HEX[8*3]+" "+DATA_HEX[8*3]+"  |"+DATA_CHAR[8]+"|"
    char hd_buffer[10 + 3 + BYTES_PER_LINE * 3 + 3 + BYTES_PER_LINE + 1 + 1];
    char *ptr_hd;
    int bytes_cur_line;

    do {
        if (buff_len > BYTES_PER_LINE) {
            bytes_cur_line = BYTES_PER_LINE;
        } else {
            bytes_cur_line = buff_len;
        }
		ptr_line = buffer;
        ptr_hd = hd_buffer;

        ptr_hd += sprintf(ptr_hd, "%p ", buffer);
        for (int i = 0; i < BYTES_PER_LINE; i ++) {
            if ((i & 7) == 0) {
                ptr_hd += sprintf(ptr_hd, " ");
            }
            if (i < bytes_cur_line) {
                ptr_hd += sprintf(ptr_hd, " %02x", ptr_line[i]);
            } else {
                ptr_hd += sprintf(ptr_hd, "   ");
            }
        }
        ptr_hd += sprintf(ptr_hd, "  |");
        for (int i = 0; i < bytes_cur_line; i ++) {
            if (isprint((int)ptr_line[i])) {
                ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
            } else {
                ptr_hd += sprintf(ptr_hd, ".");
            }
        }
        ptr_hd += sprintf(ptr_hd, "|");

        LOG_LEVEL(log_level, tag, "%s", hd_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}

