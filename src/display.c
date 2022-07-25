#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include "display.h"

typedef struct display_line_s {
	uint8_t len;
	uint8_t buffer[DISPLAY_LINE_LEN + 1];
} display_line_t;

static display_line_t lines[DISPLAY_LINE_MAX];

static display_line_t *line_idx[DISPLAY_LINE_MAX] = {0};


int display_init(){
	int i = 0;
	for(i = 0; i < DISPLAY_LINE_MAX; i++){
		line_idx[i] = &lines[i];
	}
	return 0;
}

static int display_scroll(){
	int i = 0;
	display_line_t *tmp = line_idx[0];
	oled_area_t area = {0};
	area.height = DISPLAY_FONT_SIZE;
	area.width = tmp->len * DISPLAY_FONT_WIDTH;

	for (i = 0; i < DISPLAY_LINE_MAX; i++){
		area.y = i * DISPLAY_FONT_SIZE;
		area.width = line_idx[i]->len * DISPLAY_FONT_WIDTH;
		OLED_clear_buffer(&area);
	}
	for (i = 0; i < DISPLAY_LINE_MAX - 1; i++){
		line_idx[i] = line_idx[i + 1];
	}
	tmp->len = 0;
	line_idx[i] = tmp;
	return 0;
}

static int get_empty_line(){
	int i = 0;
	for (i = 0; i < DISPLAY_LINE_MAX; i++){
		if (!line_idx[i]->len){
			return i;
		}
	}
	display_scroll();
	return DISPLAY_LINE_MAX - 1;
}

static int display_refresh(){
	int i = 0;
	for (i = 0; i < DISPLAY_LINE_MAX; i++){
		if (!line_idx[i]->len){
			continue;
		}
		OLED_ShowString(0, i * DISPLAY_FONT_SIZE, line_idx[i]->buffer, 
			DISPLAY_FONT_SIZE, 1);
	}
	OLED_Refresh();
	return 0;
}

int display_string(int line, char *str){
	int len = 0;
	if (!str){
		return 0;
	}
	len = strlen(str);
	if (line < 0 || line > DISPLAY_LINE_MAX){
		line = get_empty_line();
	}else{
		if (len < line_idx[line]->len){
			oled_area_t area = {
				.x = len * DISPLAY_FONT_WIDTH,
				.y = line * DISPLAY_FONT_SIZE,
				.height = DISPLAY_FONT_SIZE,
				.width = (line_idx[line]->len - len) * DISPLAY_FONT_WIDTH,
			};
			OLED_clear_buffer(&area);
		}
	}
	len = snprintf((char *)line_idx[line]->buffer, DISPLAY_LINE_LEN, "%s", str);
	if (len < 0){
		return -1;
	}
	line_idx[line]->len = len;
	display_refresh();
	return 0;
}

int display_printline(int line, char *fmt, ...){
	char buf[DISPLAY_LINE_LEN + 1] = {0};
	va_list vp;
	va_start(vp, fmt);
	vsnprintf(buf, DISPLAY_LINE_LEN, fmt, vp);
	va_end(vp);
	display_string(line, buf);
	return 0;
}
