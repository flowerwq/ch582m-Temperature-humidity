#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#ifndef STR
#define _STR(s)	#s
#define STR(s)		_STR(s)
#endif

#ifndef BIT0
#define BIT31   0x80000000
#define BIT30   0x40000000
#define BIT29   0x20000000
#define BIT28   0x10000000
#define BIT27   0x08000000
#define BIT26   0x04000000
#define BIT25   0x02000000
#define BIT24   0x01000000
#define BIT23   0x00800000
#define BIT22   0x00400000
#define BIT21   0x00200000
#define BIT20   0x00100000
#define BIT19   0x00080000
#define BIT18   0x00040000
#define BIT17   0x00020000
#define BIT16   0x00010000
#define BIT15   0x00008000
#define BIT14   0x00004000
#define BIT13   0x00002000
#define BIT12   0x00001000
#define BIT11   0x00000800
#define BIT10   0x00000400
#define BIT9     0x00000200
#define BIT8     0x00000100
#define BIT7     0x00000080
#define BIT6     0x00000040
#define BIT5     0x00000020
#define BIT4     0x00000010
#define BIT3     0x00000008
#define BIT2     0x00000004
#define BIT1     0x00000002
#define BIT0     0x00000001

#define BIT63    (0x80000000ULL << 32)
#define BIT62    (0x40000000ULL << 32)
#define BIT61    (0x20000000ULL << 32)
#define BIT60    (0x10000000ULL << 32)
#define BIT59    (0x08000000ULL << 32)
#define BIT58    (0x04000000ULL << 32)
#define BIT57    (0x02000000ULL << 32)
#define BIT56    (0x01000000ULL << 32)
#define BIT55    (0x00800000ULL << 32)
#define BIT54    (0x00400000ULL << 32)
#define BIT53    (0x00200000ULL << 32)
#define BIT52    (0x00100000ULL << 32)
#define BIT51    (0x00080000ULL << 32)
#define BIT50    (0x00040000ULL << 32)
#define BIT49    (0x00020000ULL << 32)
#define BIT48    (0x00010000ULL << 32)
#define BIT47    (0x00008000ULL << 32)
#define BIT46    (0x00004000ULL << 32)
#define BIT45    (0x00002000ULL << 32)
#define BIT44    (0x00001000ULL << 32)
#define BIT43    (0x00000800ULL << 32)
#define BIT42    (0x00000400ULL << 32)
#define BIT41    (0x00000200ULL << 32)
#define BIT40    (0x00000100ULL << 32)
#define BIT39    (0x00000080ULL << 32)
#define BIT38    (0x00000040ULL << 32)
#define BIT37    (0x00000020ULL << 32)
#define BIT36    (0x00000010ULL << 32)
#define BIT35    (0x00000008ULL << 32)
#define BIT34    (0x00000004ULL << 32)
#define BIT33    (0x00000002ULL << 32)
#define BIT32    (0x00000001ULL << 32)
#endif

#define LEVEL_HIGH	1
#define LEVEL_LOW	0

#ifndef BIT_SET
#define BIT_SET(data, bit)	((data) | (1UL << (bit)))
#endif
#ifndef BIT_CLEAR
#define BIT_CLEAR(data, bit) ((data) & (~(1UL << (bit))))
#endif
#ifndef BIT_GET
#define BIT_GET(data, bit)	(((data) & (1UL << (bit))) >> (bit))
#endif

#ifndef BITS_SET
#define BITS_SET(data, mask)	((data) | (mask))
#endif
#ifndef BITS_CLEAR
#define BITS_CLEAR(data, mask) ((data) & (~(mask)))
#endif
#ifndef BITS_GET
#define BITS_GET(data, mask)	((data) & (mask))
#endif

#define IO_MODE_GPIO 0
#define IO_MODE_EXPAND 1
#define IO_MODE_VALID(m) ((m) >= IO_MODE_GPIO && (m) <= IO_MODE_EXPAND)

//app version
//major version, range 0 - 255
#define APP_VERSION_MAJOR	1
//minor version, range: 0 - 255
#define APP_VERSION_MINOR	0
//fix version, range: 0 - 4095
#define APP_VERSION_FIX	0
//stage version, can be VERSION_STAGE_ALPHA, VERSION_STAGE_BETA or VERSION_STAGE_RELEASE
#ifdef RELEASE_BUILD
#define APP_VERSION_STAGE	VERSION_STAGE_RELEASE
#else
#define APP_VERSION_STAGE	VERSION_STAGE_ALPHA
#endif
//HW version
#define HW_VERSION_MAJOR	0
#define HW_VERSION_MINOR	1

//Minimum support hardware version
//major version, range 0 - 255
#define HW_VERSION_MAJOR_MIN	0
//minor version, range: 0 - 255
#define HW_VERSION_MINOR_MIN	1

#define CONSOLE_PROMPT	"TK-SENSOR"

#define HOST_UART	UART_NUM_2

#endif
