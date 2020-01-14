#include "lcdScreenDriver.h"
#include "lcdScreenDriver_internal.h"

#include "i2cInterface.h"
#include "delayAbstraction.h"

#define LCDSCREEN_I2C_CLOCK 100000L

uint8_t deviceAddress;

uint8_t displayControlOptions;
uint8_t displayModeOptions;
uint8_t numberOfRows;
uint8_t numberOfColumns;
uint8_t characterType;
uint8_t backlightState = LCDSCREEN_INTERNAL_BACKLIGHT_ON;
uint8_t currentCursorPositionColumns;
uint8_t currentCursorPositionRows;


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the MCU doesn't reset the LCD, so we
// can't assume that its in that state when the library starts.
#include <stdio.h>

uint8_t lcdScreenDriver_initialise(I2C_Registers* registers, uint8_t lcdScreenI2CAddress, uint8_t columns, uint8_t rows, uint8_t characterDotsType){
	if(lcdScreenI2CAddress == 0 || columns == 0 || rows == 0 || (characterDotsType != LCDSCREEN_TYPE_5x8DOTS && characterDotsType != LCDSCREEN_TYPE_5x10DOTS)){
		return LCDSCREEN_ERRORCODE_INVALIDPARAMS;
	}

	uint8_t errorcode = i2c_init(registers, LCDSCREEN_I2C_CLOCK);
	if(errorcode != I2C_FUNCTIONCODES_NO_ERROR){
		return errorcode;
	}
	
	deviceAddress = lcdScreenI2CAddress;
	numberOfRows = rows;
	numberOfColumns = columns;
	characterType = characterDotsType;
	return LCDSCREEN_ERRORCODE_ALL_OK;
}

void lcdScreenDriver_initialiseScreenToKnownState(void){
	backlightState = LCDSCREEN_INTERNAL_BACKLIGHT_ON;

	delayAbstraction_delayMilliseconds(50);
	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(0);
	delayAbstraction_delayMilliseconds(1000);


	//Do the waiting long twice
	lcdScreenDriverInternal_writeNibble(LCDSCREEN_INTERFACE_4BITMODE_A, LCDSCREEN_SENDING_MODE_COMMAND);
	delayAbstraction_delayMicroseconds(LCDSCREEN_INTERFACE_4BITMODE_DELAY_LONG_US);
	lcdScreenDriverInternal_writeNibble(LCDSCREEN_INTERFACE_4BITMODE_A, LCDSCREEN_SENDING_MODE_COMMAND);
	delayAbstraction_delayMicroseconds(LCDSCREEN_INTERFACE_4BITMODE_DELAY_LONG_US);

	//Then wait for a short time	
	lcdScreenDriverInternal_writeNibble(LCDSCREEN_INTERFACE_4BITMODE_A, LCDSCREEN_SENDING_MODE_COMMAND);
	delayAbstraction_delayMicroseconds(LCDSCREEN_INTERFACE_4BITMODE_DELAY_SHORT_US);

	//then finally set it to 4bit with this
	lcdScreenDriverInternal_writeNibble(LCDSCREEN_INTERFACE_4BITMODE_B, LCDSCREEN_SENDING_MODE_COMMAND);

	//Screen functionality being set
	uint8_t functionOptions = LCDSCREEN_FUNCTIONALITY_4BITMODE | LCDSCREEN_FUNCTIONALITY_1LINE | LCDSCREEN_TYPE_5x8DOTS;
	if(numberOfRows > 1){
		functionOptions |= LCDSCREEN_FUNCTIONALITY_2LINE;
	}
	if(characterType == LCDSCREEN_TYPE_5x10DOTS && numberOfRows == LCDSCREEN_FUNCTIONALITY_1LINE){
		functionOptions |= LCDSCREEN_TYPE_5x10DOTS;
	}
	lcdScreenDriver_setScreenFunctionOptions(functionOptions);
	//Turn display on, cursor and blinking off
	uint8_t controlOptions = (1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	lcdScreenDriver_setDisplayControlOptions(controlOptions);

	//clear the screen
	lcdScreenDriver_clearDisplay();

	//set the display mode to roman languages
	uint8_t displayModeOptions = (1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	lcdScreenDriver_setDisplayMode(displayModeOptions);
	lcdScreenDriver_setCursorHome();
}

void lcdScreenDriver_setBacklightOn(void){
	backlightState = LCDSCREEN_INTERNAL_BACKLIGHT_ON;
	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(0);
}

void lcdScreenDriver_setBacklightOff(void){
	backlightState = LCDSCREEN_INTERNAL_BACKLIGHT_OFF;
	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(0);
}

void lcdScreenDriver_setDisplayControlOptions(uint8_t controlOptions){
	displayControlOptions = controlOptions;
	lcdScreenDriverInternal_writeCommandByte(LCDSCREEN_COMMAND_SETDISPLAYCONTROL | displayControlOptions);
}

void lcdScreenDriver_setDisplayMode(uint8_t modeOptions){
	displayModeOptions = modeOptions;
	lcdScreenDriverInternal_writeCommandByte(displayModeOptions | LCDSCREEN_COMMAND_SETMODE);
}

void lcdScreenDriver_setScreenFunctionOptions(uint8_t functionOptions){
	lcdScreenDriverInternal_writeCommandByte(functionOptions | LCDSCREEN_FUNCTIONALITY_COMMAND);
}

void lcdScreenDriver_turnDisplayOn(void){
	displayControlOptions |= (1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	lcdScreenDriverInternal_writeCommandByte(LCDSCREEN_COMMAND_SETDISPLAYCONTROL | displayControlOptions);
}

void lcdScreenDriver_turnDisplayOff(void){
	displayControlOptions &= ~(1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	lcdScreenDriverInternal_writeCommandByte(LCDSCREEN_COMMAND_SETDISPLAYCONTROL | displayControlOptions);
}

void lcdScreenDriver_clearDisplay(void){
	lcdScreenDriverInternal_writeCommandByte(LCDSCREEN_COMMAND_CLEAR_DISPLAY);
	delayAbstraction_delayMilliseconds(2);
}

void lcdScreenDriver_setCursorHome(void){
	currentCursorPositionColumns = 0;
	currentCursorPositionRows = 0;
	lcdScreenDriverInternal_writeCommandByte(LCDSCREEN_COMMAND_MOVE_CURSOR_HOME);
	delayAbstraction_delayMilliseconds(2);
}

void lcdScreenDriver_setCursorPosition(uint8_t cursorPositionColumn, uint8_t cursorPositionRow){
	uint16_t rowOffsets[] = {0x00, 0x40};
	if(cursorPositionColumn >= numberOfColumns){
		cursorPositionColumn = numberOfColumns - 1;
	}
	if(cursorPositionRow >= numberOfRows){
		cursorPositionRow = numberOfRows - 1;
	}
	currentCursorPositionColumns = cursorPositionColumn;
	currentCursorPositionRows = cursorPositionRow;
	lcdScreenDriverInternal_writeCommandByte(LCDSCREEN_COMMAND_SET_DDRAM_ADDR | (cursorPositionColumn + rowOffsets[cursorPositionRow]));
}

void lcdScreenDriver_setCursorOff(void){
	displayControlOptions &= ~(1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	lcdScreenDriver_setDisplayControlOptions(displayControlOptions);
}

void lcdScreenDriver_setCursorOn(void){
	displayControlOptions |= (1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	lcdScreenDriver_setDisplayControlOptions(displayControlOptions);
}

void lcdScreenDriver_setBlinkOff(void){
	displayControlOptions &= ~(1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	lcdScreenDriver_setDisplayControlOptions(displayControlOptions);
}

void lcdScreenDriver_setBlinkOn(void){
	displayControlOptions |= (1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	lcdScreenDriver_setDisplayControlOptions(displayControlOptions);
}

void lcdScreenDriver_setTextFlow(uint8_t textFlowDirection){
	if(textFlowDirection){
		displayModeOptions |= (1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	}
	else{
		displayModeOptions &= ~(1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	}
	lcdScreenDriver_setDisplayMode(displayModeOptions);
}

void lcdScreenDriver_printChar(char c){
	if(c == '\n'){
		lcdScreenDriver_setCursorPosition(0, (currentCursorPositionRows+1) % numberOfRows);
		return;
	}
	if(currentCursorPositionColumns>=numberOfColumns){
		lcdScreenDriver_setCursorPosition(0, (currentCursorPositionRows+1) % numberOfRows);
	}
	lcdScreenDriverInternal_writeDataByte(c);
	currentCursorPositionColumns++;
}

void lcdScreenDriver_printString(char* string){
	char currentChar;
	while((currentChar = *(string++))){
		lcdScreenDriver_printChar(currentChar);
	}
}



// void lcdscreendriver_printChar(char c){
// 	lcdScreenDriverInternal_sendData(c, LCDSCREEN_SENDING_MODE_DATA);
// }

//##################################
//Internal applications
void lcdScreenDriverInternal_writeWithCurrentBacklightSetting(uint8_t dataToWrite){
	uint8_t errorcode = 0;
	i2c_setSlaveAddress(deviceAddress);
	errorcode = i2c_sendStartCondition();
	if(errorcode){
		// printf("error on start condition: %u\n", errorcode);
		return;
	}
	errorcode =  i2c_write(dataToWrite | backlightState);
	if(errorcode){
		// printf("error on write data: %u\n", errorcode);
		return;
	}
	i2c_sendStopCondition();
}

void lcdScreenDriverInternal_writeEnablePulse(uint8_t dataToWrite){
	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(dataToWrite | LCDSCREEN_PULSE_ENABLE_BIT);
	delayAbstraction_delayMicroseconds(1);
	
	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(dataToWrite & ~LCDSCREEN_PULSE_ENABLE_BIT);
	delayAbstraction_delayMicroseconds(50);
}

void lcdScreenDriverInternal_writeNibble(uint8_t fourBitValue, uint8_t sendingMode){
	// printf("writing nibble. Value param %u\n", fourBitValue);
	fourBitValue = fourBitValue << 4;
	// printf("writing with backlight settings %u\n", fourBitValue);
	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(fourBitValue | sendingMode);
	// printf("writing pulse\n");
	lcdScreenDriverInternal_writeEnablePulse(fourBitValue | sendingMode);
}

void lcdScreenDriverInternal_writeCommandByte(uint8_t dataToWrite){
	uint8_t highNibble = dataToWrite >> 4;
	uint8_t lowNibble = dataToWrite & 0x0F;
	lcdScreenDriverInternal_writeNibble(highNibble, LCDSCREEN_SENDING_MODE_COMMAND);
	lcdScreenDriverInternal_writeNibble(lowNibble, LCDSCREEN_SENDING_MODE_COMMAND);
}

void lcdScreenDriverInternal_writeDataByte(uint8_t dataToWrite){
	uint8_t highNibble = dataToWrite >> 4;
	uint8_t lowNibble = dataToWrite & 0x0F;
	lcdScreenDriverInternal_writeNibble(highNibble, LCDSCREEN_SENDING_MODE_DATA);
	lcdScreenDriverInternal_writeNibble(lowNibble, LCDSCREEN_SENDING_MODE_DATA);
}

/*
	Sending data with the mode 
	LCD_DATASENDING_MODE_NORMAL 1
	sends the data as a normal character where the last bit is set to 1 indicating a character data byte.

	if the mode is set to 
	LCD_DATASENDING_MODE_COMMAND 0
	it does not add anything to the byte indicating it is a command byte

	This is visible because we are sending nibbles and the lowest bit is indicating the mode.
*/


// void lcdScreenDriverInternal_sendDataNibble(uint8_t data);

// void lcdScreenDriverInternal_sendData(uint8_t dataToWrite, uint8_t mode){
// 	// printf("\nSending data %u with mode %u\n", dataToWrite, mode);
// 	uint8_t highNibble = dataToWrite >> 4;
// 	uint8_t lowNibble = dataToWrite & 0x0F;
// 	if(mode){
// 		lcdScreenDriverInternal_sendDataNibble(highNibble);
// 		lcdScreenDriverInternal_sendDataNibble(lowNibble);
// 	}
// 	else{
// 		printf("you are not meant to be here WTF\n\n\n");
// 	}

// }

// void lcdScreenDriverInternal_sendDataNibble(uint8_t data){
// 	// printf("writing data nibble. Value param %u\n", data);
// 	data = data << 4;
// 	// printf("writing data nibble with backlight settings %u\n", data);
// 	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(data | LCDSCREEN_SENDING_MODE_DATA);
// 	// printf("writing pulse\n");
// 	lcdScreenDriverInternal_writeEnablePulse(data | LCDSCREEN_SENDING_MODE_DATA);
// }