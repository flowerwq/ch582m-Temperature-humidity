#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include "stdint.h"
typedef struct ring_buffer{
	uint8_t *head;
	uint8_t *tail;
	uint16_t dlen;
	uint16_t content_len;
	uint8_t *content;
}ringbuffer_t;

uint8_t ringbuffer_init(ringbuffer_t     *rb, uint8_t *content, uint16_t clen);
uint16_t ringbuffer_write(ringbuffer_t *rb, uint8_t *buf, uint16_t len);
uint16_t ringbuffer_read(ringbuffer_t *rb, uint8_t *buf, uint16_t len);
uint16_t ringbuffer_get_length(ringbuffer_t *rb);
void ringbuffer_drop(ringbuffer_t *rb, uint16_t len);

#endif
