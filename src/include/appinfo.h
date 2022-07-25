#ifndef __APPINFO_H__
#define __APPINFO_H__

#include "stdint.h"
#pragma pack(push)
#pragma pack(2)
typedef struct appinfo_s{
	uint16_t magic;
	uint16_t vid;
	uint16_t pid;
	uint32_t version;
	uint8_t buildtime[32];
	uint16_t hw_version;
	uint16_t hw_version_min;
}appinfo_t;
#pragma pack(pop)

#define APP_MAGIC	0x3736
#define APPINFO_OFFSET	4

#define VID_CG	1
#define PID_BOOT	0xffff
#define PID_TKS_TRH	1	//temperature&humidity
#define PID_TKS_WL	2	//water level
#define PID_TKS_WIDS	3//water-intrusion detecting sensor
#define PID_TKS_SF6O2	4	//SF6+O2
#define PID_TKS_O3		5	//O3
#define PID_TKS_TEMP_RFID	6	//Temperature(RFID)
#define PID_TKS_TEMP_BLE	7	//Temperature(BLE)
#define PID_TKS_ACS		8	//Access Control System
#define PID_TKS_SMOKE	9	//Smoke Detect
#define PID_TKS_TOPO	10	//power network topology
#define PID_TKS_NOISE	11	//noise detecting sensor
#define PID_TKS_WEATHER	12	//weather

const appinfo_t *appinfo_get();
#endif
