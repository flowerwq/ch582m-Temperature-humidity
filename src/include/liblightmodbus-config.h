#ifndef __LIBLIGHTMODBUS_CONFIG_H__
#define __LIBLIGHTMODBUS_CONFIG_H__

//#define LIGHTMODBUS_IMPL	//Include implementation
#define LIGHTMODBUS_SLAVE	//Includes slave part of the library
#define LIGHTMODBUS_SLAVE_FULL	//Includes slave part of the library and adds all functions to modbusSlaveDefaultFunctions
//#define LIGHTMODBUS_FxxS	//Adds function xx to modbusSlaveDefaultFunctions
#define LIGHTMODBUS_F03S	//03:read holding registers
#define LIGHTMODBUS_F06S	//06:write single register
#define LIGHTMODBUS_F16S	//16:write multipule registers

//#define LIGHTMODBUS_MASTER		//Includes master part of the library
//#define LIGHTMODBUS_FxxM			//Adds function xx to modbusMasterDefaultFunctions
//#define LIGHTMODBUS_MASTER_FULL	//Includes master part of the library and adds all functions to modbusMasterDefaultFunctions
//#define LIGHTMODBUS_FULL		//Equivalent of both LIGHTMODBUS_SLAVE_FULL and LIGHTMODBUS_MASTER_FULL
//#define LIGHTMODBUS_DEBUG		//Includes some debugging utilities
//#define LIGHTMODBUS_MASTER_OMIT_REQUEST_CRC	//Omits request CRC calculation for request on master side
//#define LIGHTMODBUS_WARN_UNUSED	//Compiler attribute to warn about unused return value. __attribute__((warn_unused_result)) by default
//#define LIGHTMODBUS_ALWAYS_INLINE //Compiler attribute to always inline a function. __attribute__((always_inline)) by default
#define LIGHTMODBUS_DEBUG

#endif
