#ifndef _I2CINTERFACE_H
#define _I2CINTERFACE_H

#include <stdint.h>

typedef struct{
	volatile uint8_t* baudrateRegister;
	volatile uint8_t* statusRegister;
	volatile uint8_t* controlRegister;
	volatile uint8_t* dataRegister;
	volatile uint8_t* pullUpPinsDataDirectionRegister;
	volatile uint8_t* pullUpPinsPortRegister;
	volatile uint8_t sdaPin;
	volatile uint8_t sclPin;
}I2C_Registers;

#define I2C_FUNCTIONCODES_NO_ERROR 0
#define I2C_FUNCTIONCODES_INVALID_PARAMS 1

#define I2C_CODES_NO_ERROR 0
#define I2C_CODES_INVALID_PARAMS 1
#define I2C_CODES_START_CONDITION_FAILED 2
#define I2C_CODES_SLAVE_ADDR_TRANSMIT_FAILED 3
#define I2C_CODES_DATA_TRANSMIT_FAILED 4
#define I2C_CODES_DATA_READ_FAILED 5


uint8_t i2c_init(I2C_Registers* i2cRegisters, uint32_t clockspeed);
void i2c_setSlaveAddress(uint8_t addressToSet);
uint8_t i2c_sendStartCondition(void);
void i2c_sendStopCondition(void);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_writeBytes(uint8_t* data, uint8_t dataLength);
uint8_t i2c_readBytes(uint8_t* dataBuffer, uint16_t dataLength);
uint8_t i2c_read();

int8_t i2c_writeToRegister(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, uint16_t dataLength);
int8_t i2c_readFromRegister(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* dataBuffer, uint16_t dataLength);

#endif // _I2CINTERFACE_H
