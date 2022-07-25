#include <stdio.h>
#include <string.h>
#include "stdint.h"
#include "utils.h"
#include "uid.h"
#include "CH58x_common.h"

static uint8_t uid_read = 0;
static uint8_t uid[UID_LENGTH + 1];
#define UID_ADDR_BASE	0x0127b
const uint8_t *get_uid(){
	if (!uid_read){
		GET_UNIQUE_ID(uid);
		uid_read = 1;
	}
	return uid;
}
