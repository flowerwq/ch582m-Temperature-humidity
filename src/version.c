#include <stdio.h>
#include <string.h>
#include "stdint.h"

#include "version.h"
int version_str(uint32_t version, char *buf, int len){
	if (!buf || len <= 0){
		return -1;
	}
	uint32_t stage = VERSION_GET_STAGE(version);
	switch(stage){
		case VERSION_STAGE_ALPHA:
			snprintf(buf, len, "%lu.%lu.%lu%c", VERSION_GET_MAJOR(version),
				VERSION_GET_MINOR(version), VERSION_GET_FIX(version), 
				VERSION_STAGE_ALPHA_CODE);
			break;
		case VERSION_STAGE_BETA:
			snprintf(buf, len, "%lu.%lu.%lu%c", VERSION_GET_MAJOR(version),
				VERSION_GET_MINOR(version), VERSION_GET_FIX(version), 
				VERSION_STAGE_BETA_CODE);
			break;
		case VERSION_STAGE_RELEASE:
			snprintf(buf, len, "%lu.%lu.%lu%c", VERSION_GET_MAJOR(version),
				VERSION_GET_MINOR(version), VERSION_GET_FIX(version), 
				VERSION_STAGE_RELEASE_CODE);
			break;
		default:
			snprintf(buf, len, "%lu.%lu.%lu", VERSION_GET_MAJOR(version),
				VERSION_GET_MINOR(version), VERSION_GET_FIX(version));
	}
	return 0;
}
