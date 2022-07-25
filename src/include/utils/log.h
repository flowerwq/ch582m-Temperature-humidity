#ifndef __UTILS_LOG__
#define __UTILS_LOG__

#include "app_config.h"

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL LOG_LEVEL_DEBUG
#endif

typedef enum {
    LOG_LEVEL_NONE,       /*!< No log output */
    LOG_LEVEL_ERROR,      /*!< Critical errors, software module can not recover on its own */
    LOG_LEVEL_WARN,       /*!< Error conditions from which recovery measures have been taken */
    LOG_LEVEL_INFO,       /*!< Information messages which describe normal flow of events */
    LOG_LEVEL_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    LOG_LEVEL_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} log_level_t;

#if CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V
#else //CONFIG_LOG_COLORS
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " %s: " format LOG_RESET_COLOR "\r\n"
#define LOG_DEBUG(TAG, fmt, ...) 	if (LOG_LOCAL_LEVEL >= LOG_LEVEL_DEBUG){PRINT(LOG_FORMAT(D, fmt), TAG, ##__VA_ARGS__);}
#define LOG_ERROR(TAG, fmt, ...)	if (LOG_LOCAL_LEVEL >= LOG_LEVEL_ERROR){PRINT(LOG_FORMAT(E, fmt), TAG, ##__VA_ARGS__);}
#define LOG_INFO(TAG, fmt, ...)		if (LOG_LOCAL_LEVEL >= LOG_LEVEL_INFO){PRINT(LOG_FORMAT(I, fmt), TAG, ##__VA_ARGS__);}
#define LOG_VERBOSE(TAG, fmt, ...)		if (LOG_LOCAL_LEVEL >= LOG_LEVEL_VERBOSE){PRINT(LOG_FORMAT(V, fmt), TAG, ##__VA_ARGS__);}
#define LOG_WARN(TAG, fmt, ...)		if (LOG_LOCAL_LEVEL >= LOG_LEVEL_WARN){PRINT(LOG_FORMAT(W, fmt), TAG, ##__VA_ARGS__);}

#define LOG_LEVEL(level, tag, format, ...) do {                     \
    if (level == LOG_LEVEL_ERROR )          { LOG_ERROR(tag, format, ##__VA_ARGS__); } \
    else if (level==LOG_LEVEL_WARN )      { LOG_WARN(tag, format, ##__VA_ARGS__); } \
    else if (level==LOG_LEVEL_DEBUG )     { LOG_DEBUG(tag, format, ##__VA_ARGS__); } \
    else if (level==LOG_LEVEL_VERBOSE )   { LOG_VERBOSE(tag, format, ##__VA_ARGS__); } \
    else                                { LOG_INFO(tag, format, ##__VA_ARGS__); } \
} while(0)

void log_buffer_hexdump(const char *tag, const void *buffer, 
	uint16_t buff_len, log_level_t log_level);
void log_buffer_hex(const char *tag, const void *buffer, uint16_t buff_len,
	log_level_t log_level);

#endif
