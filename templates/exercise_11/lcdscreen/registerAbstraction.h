#ifndef _REGISTERABSTRACTION_H
#define _REGISTERABSTRACTION_H

#include <stdint.h>

void abstraction_setRegisterToValue(volatile uint8_t* targetRegister, uint8_t valueToSet);
void abstraction_setRegisterBitsHigh(volatile uint8_t* targetRegister, uint8_t bitmask);
void abstraction_setRegisterBitsLow(volatile uint8_t*  targetRegister, uint8_t bitmask);

uint8_t abstraction_isBitSet(volatile uint8_t* targetRegister, uint8_t bitToRead);

uint8_t abstraction_getRegisterValue(volatile uint8_t* targetRegister);

#endif // _REGISTERABSTRACTION_H
