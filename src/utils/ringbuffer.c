#include <stdio.h>
#include <string.h>
#include "utils/ringbuffer.h"
#include "utils.h"

uint8_t ringbuffer_init(ringbuffer_t     *rb, uint8_t *content, uint16_t clen){
	if (!rb || !content || clen <= 0){
		return -1;
	}
	memset(rb, 0, sizeof(ringbuffer_t));
	memset(content, 0, clen);
	rb->content = content;
	rb->head = rb->tail = content;
	rb->content_len = clen;
	return 0;
}

uint16_t ringbuffer_get_length(ringbuffer_t *rb){
	return rb ? rb->dlen : 0;
}

uint16_t ringbuffer_write(ringbuffer_t *rb, uint8_t *buf, uint16_t len){
	uint16_t bytes_write, bytes_remain, tmp;
	uint8_t *idx = buf;
	if (!rb || !rb->content || !buf){
		return -1;
	}
	if (len <= 0){
		return 0;
	}
	if (rb->dlen >= rb->content_len){
		return 0;
	}
	bytes_remain = bytes_write = MIN(len, rb->content_len - rb->dlen);
	tmp = rb->content + rb->content_len - rb->tail;
	if (bytes_write > tmp){
		memcpy(rb->tail, idx, tmp);
		idx += tmp;
		bytes_remain -= tmp;
		rb->tail = rb->content;
	}
	memcpy(rb->tail, idx, bytes_remain);
	rb->tail += bytes_remain;
	rb->dlen += bytes_write;
	return bytes_write;
}

uint16_t ringbuffer_read(ringbuffer_t *rb, uint8_t *buf, uint16_t len){
	uint16_t bytes_read, bytes_remain, tmp;
	uint8_t *idx = buf;
	if (!rb || !rb->content || !buf){
		return -1;
	}
	if (rb->dlen <= 0 || len <= 0){
		return 0;
	}
	bytes_remain = bytes_read = MIN(rb->dlen, len);
	tmp = rb->content + rb->content_len - rb->head;
	if (bytes_read > tmp){
		memcpy(idx, rb->head, tmp);
		rb->head = rb->content;
		idx += tmp;
		bytes_remain -= tmp;
	}
	memcpy(idx, rb->head, bytes_remain);
	rb->head += bytes_remain;
	rb->dlen -= bytes_read;
	return bytes_read;
}

void ringbuffer_drop(ringbuffer_t *rb, uint16_t len){
	uint16_t bytes_read, bytes_remain, tmp;
	if (!rb || !rb->content){
		return;
	}
	if (rb->dlen <= 0 || len <= 0){
		return;
	}
	bytes_remain = bytes_read = MIN(rb->dlen, len);
	tmp = rb->content + rb->content_len - rb->head;
	if (bytes_read > tmp){
		rb->head = rb->content;
		bytes_remain -= tmp;
	}
	rb->head += bytes_remain;
	rb->dlen -= bytes_read;
}

