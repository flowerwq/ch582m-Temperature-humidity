#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "oled.h"

#define DISPLAY_FONT_SIZE	16
#define DISPLAY_FONT_WIDTH	8

#define DISPLAY_LINE_MAX	(OLED_Y_MAX / DISPLAY_FONT_SIZE)
#define DISPLAY_LINE_LEN	(OLED_X_MAX / DISPLAY_FONT_WIDTH)

#define DISPLAY_LAST_LINE	(DISPLAY_LINE_MAX - 1)
#define DISPLAY_ANY_LINE	-1

#define DISPLAY_PRINT(fmt, ...)	display_printline(DISPLAY_ANY_LINE, fmt, ##__VA_ARGS__)

int display_init();
int display_string(int line, char *str);
int display_printline(int line, char *fmt, ...);

#endif
