#include "appinfo.h"
#include "version.h"
static appinfo_t appinfo __attribute__((section(".appinfo"))) = {
	APP_MAGIC,
	VID_CG,
	PID_TKS_TRH,
	CURRENT_VERSION(),
	__DATE__ __TIME__,
	HW_VERSION(),
	HW_VERSION_MIN(),
};

const appinfo_t *appinfo_get(){
	return &appinfo;
}
