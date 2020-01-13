#ifndef _LCDSCREENDRIVER_H
#define _LCDSCREENDRIVER_H

#include <stdint.h>
#include "i2cInterface.h"

#define LCDSCREEN_ERRORCODE_ALL_OK 0x00
#define LCDSCREEN_ERRORCODE_INVALIDPARAMS 0x01


#define LCDSCREEN_TYPE_5x8DOTS 0x00
#define LCDSCREEN_TYPE_5x10DOTS 0x01

#define LCDSCREEN_CONTROL_DISPLAY_ON_BIT 2
#define LCDSCREEN_CONTROL_CURSOR_ON_BIT 1
#define LCDSCREEN_CONTROL_BLINK_ON_BIT 0


#define LCDSCREEN_TEXTFLOW_LEFTTORIGHT 1
#define LCDSCREEN_TEXTFLOW_RIGHTTOLEFT 0

#define LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT 1 //if bit set to one it is left to right, set to zero it is right to left
#define LCDSCREEN_MODE_SHIFTINCREMENT_BIT 0 //if set to 1 it is Increment, if set to zero it is decrement

uint8_t lcdScreenDriver_initialise(I2C_Registers* registers, uint8_t lcdScreenI2CAddress, uint8_t charactersPerRow, uint8_t numberOfRows, uint8_t screenType);
void lcdScreenDriver_initialiseScreenToKnownState(void);
void lcdScreenDriver_setDisplayControlOptions(uint8_t controlOptions);
void lcdScreenDriver_setDisplayMode(uint8_t modeOptions);
void lcdScreenDriver_setScreenFunctionOptions(uint8_t functionOptions);
void lcdScreenDriver_setBacklightOn(void);
void lcdScreenDriver_setBacklightOff(void);
void lcdScreenDriver_turnDisplayOn(void);
void lcdScreenDriver_turnDisplayOff(void);
void lcdScreenDriver_setCursorPosition(uint8_t cursorPositionColumn, uint8_t cursorPositionRow);
void lcdScreenDriver_setCursorOff(void);
void lcdScreenDriver_setCursorOn(void);
void lcdScreenDriver_setBlinkOff(void);
void lcdScreenDriver_setBlinkOn(void);
void lcdScreenDriver_setTextFlow(uint8_t textFlowDirection);
void lcdScreenDriver_clearDisplay(void);
void lcdScreenDriver_setCursorHome(void);
void lcdScreenDriver_printChar(char c);
void lcdScreenDriver_printString(char* string);

#endif // _LCDSCREENDRIVER_H
