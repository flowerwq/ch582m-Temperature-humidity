#ifndef __WORKTIME_H__
#define __WIRKTIME_H__

#include "stdint.h"
typedef uint64_t worktime_t;

int worktime_init();
worktime_t worktime_get();
worktime_t worktime_since(worktime_t from);

#endif
