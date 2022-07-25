#ifndef __VERSION_H__
#define __VERSION_H__

#include "app_config.h"
#include "stdint.h"

//hw version |major	|minor	|
//hw version |***8**|***8***|

#define MK_HW_VERSION(major, minor)	((((major) & 0xffUL) << 8) | \
	((minor) & 0xffUL))

#define HW_VERSION()	MK_HW_VERSION(HW_VERSION_MAJOR, HW_VERSION_MINOR)
#define HW_VERSION_MIN()	MK_HW_VERSION(HW_VERSION_MAJOR_MIN, HW_VERSION_MINOR_MIN)

// version |major   |minor    |fix       |stage|
// version |****8***|****8****|*****12****|**4**|
#define VERSION_STAGE_ALPHA	1
#define VERSION_STAGE_ALPHA_CODE	'a'
#define VERSION_STAGE_ALPHA_STR	"a"
#define VERSION_STAGE_BETA	2
#define VERSION_STAGE_BETA_CODE	'b'
#define VERSION_STAGE_BETA_STR	"b"
#define VERSION_STAGE_RELEASE	0x0fU
#define VERSION_STAGE_RELEASE_CODE	'r'
#define VERSION_STAGE_RELEASE_STR	"r"

#define VERSION_GET_MAJOR(num)	((num >> 24) & 0xffUL)
#define VERSION_GET_MINOR(num)	((num >> 16) & 0xffUL)
#define VERSION_GET_FIX(num)	((num >> 4) & 0xfffUL)
#define VERSION_GET_STAGE(num) (num & 0x0fUL)
#define MK_VERSION_NUM(major, minor, fix, stage)	(((major & 0xffUL) << 24) | \
	((minor & 0xffUL) << 16) | ((fix & 0xfffUL) << 4) | (stage & 0x0fUL))

#define VERSION_MAJOR	APP_VERSION_MAJOR
#define VERSION_MINOR	APP_VERSION_MINOR
#define VERSION_FIX		APP_VERSION_FIX
#define VERSION_STAGE	APP_VERSION_STAGE
#if VERSION_STAGE == VERSION_STAGE_ALPHA
	#define VERSION_STAGE_CODE	VERSION_STAGE_ALPHA_CODE
	#define VERSION_STAGE_STR	VERSION_STAGE_ALPHA_STR
#elif VERSION_STAGE == VERSION_STAGE_BETA
	#define VERSION_STAGE_CODE	VERSION_STAGE_BETA_CODE
	#define VERSION_STAGE_STR	VERSION_STAGE_BETA_STR
#elif VERSION_STAGE == VERSION_STAGE_RELEASE
	#define VERSION_STAGE_CODE	VERSION_STAGE_RELEASE_CODE
	#define VERSION_STAGE_STR	VERSION_STAGE_RELEASE_STR
#else
	#define VERSION_STAGE_CODE	''
	#define VERSION_STAGE_STR	""
#endif

#ifndef STR
#define _STR(n)	#n
#define STR(n)	_STR(n)
#endif

#define CURRENT_VERSION()	MK_VERSION_NUM(VERSION_MAJOR, VERSION_MINOR, VERSION_FIX, VERSION_STAGE)
#define CURRENT_VERSION_STR()	STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_FIX) VERSION_STAGE_STR

int version_str(uint32_t version, char * buf, int len);
#endif

