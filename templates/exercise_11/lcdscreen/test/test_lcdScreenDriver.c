#include "unity.h"

#include "lcdScreenDriver.h"
#include "lcdScreenDriver_internal.h"

#include "mock_i2cInterface.h"
#include "mock_delayAbstraction.h"

void setUp(void){}

void tearDown(void){}

//example registers needed by the i2c library. 
uint8_t i2cTestBaudRateRegister = 0xff;
uint8_t i2cTestStatusRegister = 0xff;
uint8_t i2cTestControlRegister = 0xff;
uint8_t i2cTestDataRegister = 0xff;
uint8_t powerReductionTestRegister = 0xff;

I2C_Registers i2c_Registers_struct = {&i2cTestBaudRateRegister, &i2cTestStatusRegister, &i2cTestControlRegister, 
		&i2cTestDataRegister};


//#####################################################
//test helper functions signatures
//#####################################################

void helper_ignoreI2CInteraction(void);
void helper_prepareWriteIncludingBacklight(uint8_t lcdScreenI2CAddress, uint8_t data, uint8_t backlightState);
void helper_prepareWriteEnablePulse(uint8_t lcdScreenI2CAddress, uint8_t data);
void helper_prepareWriteNibble(uint8_t lcdScreenI2CAddress, uint8_t data, uint8_t sendingMode);
void helper_prepareWriteCommandByte(uint8_t lcdScreenI2CAddress, uint8_t data);
void helper_prepareSetDisplayControlOptions(uint8_t controlOptions);
void helper_prepareSetScreenFunctionOptions(uint8_t functionOptions);
void helper_prepareClearDisplay(void);
void helper_prepareSetDisplayModeOptions(uint8_t modeOptions);
void helper_prepareSetCursorToHome(void);
void helper_prepareSetCursorPosition(uint8_t lcdScreenI2CAddress, uint8_t cursorPositionColumn, uint8_t cursorPositionRow);
void helper_prepareWriteDataByte(uint8_t lcdScreenI2CAddress, uint8_t data);

/*#####################################################
Tests
#####################################################*/
/*
	lcd screen needs some i2c interface 
	lcd screen needs an address under which it is reached

	if lcd screen is compiled together with 
*/

//############################################
//internal helper functions to test
//These functions are essential for all following public functions.
//commands are send by settting the mode bit (lowest bit in the actual byte being sent to) to zero
//To send a data byte, aka some characters, that mode bit is set to one. 
//it has to be set when sending a nibble and then sent for both lower and higher nibble
void test_lcdScreenDriver_internal_writeIncludingBacklight(void){
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t dataToWrite = 42;
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, dataToWrite, LCDSCREEN_INTERNAL_BACKLIGHT_ON);

	lcdScreenDriverInternal_writeWithCurrentBacklightSetting(dataToWrite);
}

void test_lcdScreenDriver_internal_writeEnablePulse(void){
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t someDummyData = 23;
	helper_prepareWriteEnablePulse(lcdScreenI2CAddress, someDummyData);

	lcdScreenDriverInternal_writeEnablePulse(someDummyData);
}

void test_lcdScreenDriver_internal_writeNibbleWithSendingModeCommand(void){
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t fourBitValue = 0x03;
	uint8_t dataToBeSent = fourBitValue << 4;
	uint8_t sendingMode = LCDSCREEN_SENDING_MODE_COMMAND;
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, dataToBeSent | sendingMode, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	helper_prepareWriteEnablePulse(lcdScreenI2CAddress, dataToBeSent | sendingMode);

	lcdScreenDriverInternal_writeNibble(fourBitValue, sendingMode);
}

void test_lcdScreenDriver_internal_writeNibbleWithSendingModeData(void){
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t fourBitValue = 0x03;
	uint8_t dataToBeSent = fourBitValue << 4;
	uint8_t sendingMode = LCDSCREEN_SENDING_MODE_DATA;
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, dataToBeSent | sendingMode, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	helper_prepareWriteEnablePulse(lcdScreenI2CAddress,dataToBeSent | sendingMode);

	lcdScreenDriverInternal_writeNibble(fourBitValue, sendingMode);
}

void test_lcdScreenDriver_internal_writeCommandByte(void){
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t data = 0xAB;
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, data);
	lcdScreenDriverInternal_writeCommandByte(data);
}

void test_lcdScreenDriver_internal_writeDataByte(void){
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t data = 0xBC;
	uint8_t highNibble = data >> 4;
	uint8_t lowNibble = data & 0x0F;
	helper_prepareWriteNibble(lcdScreenI2CAddress, highNibble, LCDSCREEN_SENDING_MODE_DATA);
	helper_prepareWriteNibble(lcdScreenI2CAddress, lowNibble, LCDSCREEN_SENDING_MODE_DATA);

	lcdScreenDriverInternal_writeDataByte(data);
}

//############################################
//Public API tests
//initialise the library
void test_lcdScreenDriver_initialiseWithValidParametersReturnsZero(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;
	i2c_init_ExpectAndReturn(&i2c_Registers_struct, 100000L, 0);
	
	uint8_t actualerrorCode = lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);
	TEST_ASSERT_EQUAL_UINT8(LCDSCREEN_ERRORCODE_ALL_OK, actualerrorCode);
}

void test_lcdScreenDriver_initialiseWithNullPointerRegistersReturnsI2CInterfaceError(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;

	i2c_init_ExpectAndReturn(0, 100000, I2C_FUNCTIONCODES_INVALID_PARAMS);
	uint8_t actualerrorCode = lcdScreenDriver_initialise(0, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);
	TEST_ASSERT_EQUAL_UINT8(LCDSCREEN_ERRORCODE_INVALIDPARAMS, actualerrorCode);
}

void test_lcdScreenDriver_initialiseWithZeroI2CAddressReturnsOne(void){
	//I2C address zero is reserved for broadcasts. A device can never have the address 0x00
	uint8_t lcdScreenI2CAddress = 0;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;

	uint8_t actualerrorCode = lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);
	TEST_ASSERT_EQUAL_UINT8(LCDSCREEN_ERRORCODE_INVALIDPARAMS, actualerrorCode);
}

void test_lcdScreenDriver_initialiseWithZeroCharactersPerRowReturnsOne(void){
	//I2C address zero is reserved for broadcasts. A device can never have the address 0x00
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 0;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;

	uint8_t actualerrorCode = lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);
	TEST_ASSERT_EQUAL_UINT8(LCDSCREEN_ERRORCODE_INVALIDPARAMS, actualerrorCode);
}

void test_lcdScreenDriver_initialiseWithZeroNumberOfRowsReturnsOne(void){
	//I2C address zero is reserved for broadcasts. A device can never have the address 0x00
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 0;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;

	uint8_t actualerrorCode = lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);
	TEST_ASSERT_EQUAL_UINT8(LCDSCREEN_ERRORCODE_INVALIDPARAMS, actualerrorCode);
}

void test_lcdScreenDriver_initialiseWithInvalidLcdScreenTypeReturnsOne(void){
	//I2C address zero is reserved for broadcasts. A device can never have the address 0x00
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = 0x42;

	uint8_t actualerrorCode = lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);
	TEST_ASSERT_EQUAL_UINT8(LCDSCREEN_ERRORCODE_INVALIDPARAMS, actualerrorCode);
}

void test_lcdScreenDriver_setBacklightOn(void){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, 0, LCDSCREEN_INTERNAL_BACKLIGHT_ON);

	lcdScreenDriver_setBacklightOn();
}

void test_lcdScreenDriver_setBacklightOff(void){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, 0, LCDSCREEN_INTERNAL_BACKLIGHT_OFF);

	lcdScreenDriver_setBacklightOff();
}

void test_lcdScreenDriver_setDisplayControlOptions(void){
	//return to the state as created by the init
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, 0, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	lcdScreenDriver_setBacklightOn();

	uint8_t controlOptions = (1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT) | (1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	controlOptions &= ~(1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	helper_prepareSetDisplayControlOptions(controlOptions);
	lcdScreenDriver_setDisplayControlOptions(controlOptions);
}

void test_lcdScreenDriver_turnDisplayOn(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t previousOptionSet = 1 << LCDSCREEN_CONTROL_BLINK_ON_BIT;
	previousOptionSet &= ~((1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT) | (1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT));

	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SETDISPLAYCONTROL | previousOptionSet);
	lcdScreenDriver_setDisplayControlOptions(previousOptionSet);
	
	//actual test
	uint8_t controlOptionExpected = previousOptionSet | (1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SETDISPLAYCONTROL | controlOptionExpected);
	lcdScreenDriver_turnDisplayOn();
}

void test_lcdScreenDriver_turnDisplayOff(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t previousOptionSet = (1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SETDISPLAYCONTROL | previousOptionSet);
	lcdScreenDriver_setDisplayControlOptions(previousOptionSet);
	
	//actual test
	uint8_t controlOptionExpected = previousOptionSet & ~(1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SETDISPLAYCONTROL | controlOptionExpected);
	lcdScreenDriver_turnDisplayOff();
}

void test_lcdScreenDriver_clearDisplay(void){
	helper_prepareClearDisplay();
	lcdScreenDriver_clearDisplay();
}

void test_lcdScreenDriver_setDisplayModeOptions(void){
	uint8_t displaymode = (1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	helper_prepareSetDisplayModeOptions(displaymode);

	lcdScreenDriver_setDisplayMode(displaymode);
}

void test_lcdScreenDriver_setCursorToHome(void){
	helper_prepareSetCursorToHome();
	lcdScreenDriver_setCursorHome();
}

void test_lcdScreenDriver_setScreenFunctionOptions(void){
	uint8_t lcdScreenFunctionOptions = LCDSCREEN_FUNCTIONALITY_4BITMODE | LCDSCREEN_FUNCTIONALITY_2LINE | LCDSCREEN_TYPE_5x8DOTS;
	helper_prepareSetScreenFunctionOptions(lcdScreenFunctionOptions);

	lcdScreenDriver_setScreenFunctionOptions(lcdScreenFunctionOptions);
}

void test_lcdScreenDriver_initialiseScreenToKnownState(void){
	//After initialising the library, the initScreen should use the set parameters
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;
	i2c_init_ExpectAndReturn(&i2c_Registers_struct, 100000L, 0);
	
	lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);

	//###### Prepare initialise Screen parts
	//wait until device is powered up correctly
	delayAbstraction_delayMilliseconds_Expect(50);
	
	//initialise backlight with current backlight settings
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, 0, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	delayAbstraction_delayMilliseconds_Expect(1000);
	
	for(uint8_t i = 0; i < 2; i++){
		//Set the screen to 4bit mode two times to be very sure
		helper_prepareWriteNibble(lcdScreenI2CAddress, LCDSCREEN_INTERFACE_4BITMODE_A, LCDSCREEN_SENDING_MODE_COMMAND);
		delayAbstraction_delayMicroseconds_Expect(LCDSCREEN_INTERFACE_4BITMODE_DELAY_LONG_US);
	}
	//set it a third time but with smaller delay
	helper_prepareWriteNibble(lcdScreenI2CAddress, LCDSCREEN_INTERFACE_4BITMODE_A, LCDSCREEN_SENDING_MODE_COMMAND);
	delayAbstraction_delayMicroseconds_Expect(LCDSCREEN_INTERFACE_4BITMODE_DELAY_SHORT_US);
	
	//finalize setting to 4 bit mode. no delay
	helper_prepareWriteNibble(lcdScreenI2CAddress, LCDSCREEN_INTERFACE_4BITMODE_B, LCDSCREEN_SENDING_MODE_COMMAND);
	
	uint8_t functionOptions = LCDSCREEN_FUNCTIONALITY_4BITMODE | LCDSCREEN_FUNCTIONALITY_2LINE | LCDSCREEN_TYPE_5x8DOTS;
	helper_prepareSetScreenFunctionOptions(functionOptions);

	uint8_t controlOptions = (1 << LCDSCREEN_CONTROL_DISPLAY_ON_BIT);
	helper_prepareSetDisplayControlOptions(controlOptions);
	
	helper_prepareClearDisplay();

	uint8_t displayModeOptions = (1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	helper_prepareSetDisplayModeOptions(displayModeOptions);

	helper_prepareSetCursorToHome();

	lcdScreenDriver_initialiseScreenToKnownState();
}

void test_lcdScreenDriver_setCursorPositionWithValidParams(void){
	//prepare library to be initialised
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t charactersPerRow = 16;
	uint8_t numberOfRows = 2;
	uint8_t screenType_NumberOfDotsPerCharacter = LCDSCREEN_TYPE_5x8DOTS;
	i2c_init_ExpectAndReturn(&i2c_Registers_struct, 100000L, 0);
	uint8_t actualerrorCode = lcdScreenDriver_initialise(&i2c_Registers_struct, lcdScreenI2CAddress, charactersPerRow, numberOfRows, screenType_NumberOfDotsPerCharacter);

	//test
	uint8_t cursorPositionRow = 0;
	uint8_t cursorPositionColumn = 3;
	uint16_t rowOffsets[] = {0x00, 0x40}; //, 0x14, 0x54}; //From the original implementation.
	uint8_t memoryAddress = LCDSCREEN_COMMAND_SET_DDRAM_ADDR | cursorPositionColumn + rowOffsets[cursorPositionRow];
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, memoryAddress);

	lcdScreenDriver_setCursorPosition(cursorPositionColumn, cursorPositionRow);
}

void test_lcdScreenDriver_setCursorPositionInvalidRowPositionHandledAsLargestRowPositionPossible(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t cursorPositionRow = 4;
	uint8_t cursorPositionColumn = 0;
	uint8_t expectedPositionRow = 1;
	uint8_t rowOffsets[] = {0x00, 0x40};
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SET_DDRAM_ADDR | cursorPositionColumn + rowOffsets[expectedPositionRow]);

	lcdScreenDriver_setCursorPosition(cursorPositionColumn, cursorPositionRow);
}

void test_lcdScreenDriver_setCursorPositionInvalidColumnHandledAsLargestColumnPossible(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t cursorPositionRow = 1;
	uint8_t cursorPositionColumn = 22;
	uint8_t expectedPositionColumn = 15;
	uint8_t rowOffsets[] = {0x00, 0x40, 0xFA, 0xAF};
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SET_DDRAM_ADDR | expectedPositionColumn + rowOffsets[cursorPositionRow]);

	lcdScreenDriver_setCursorPosition(cursorPositionColumn, cursorPositionRow);
}

void test_lcdScreenDriver_setCursorOff(void){
	uint8_t previousDisplayControlValue = (1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT) | (1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	helper_prepareSetDisplayControlOptions(previousDisplayControlValue);
	lcdScreenDriver_setDisplayControlOptions(previousDisplayControlValue);

	uint8_t expectedNewDisplayControlValue = previousDisplayControlValue & ~(1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	helper_prepareSetDisplayControlOptions(expectedNewDisplayControlValue);

	lcdScreenDriver_setCursorOff();
}

void test_lcdScreenDriver_setCursorOn(void){
	uint8_t previousDisplayControlValue = (1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	helper_prepareSetDisplayControlOptions(previousDisplayControlValue);
	lcdScreenDriver_setDisplayControlOptions(previousDisplayControlValue);

	uint8_t expectedNewDisplayControlValue = previousDisplayControlValue | (1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	helper_prepareSetDisplayControlOptions(expectedNewDisplayControlValue);

	lcdScreenDriver_setCursorOn();
}

void test_lcdScreenDriver_setBlinkOff(void){
	uint8_t previousDisplayControlValue = (1 << LCDSCREEN_CONTROL_BLINK_ON_BIT) | (1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	helper_prepareSetDisplayControlOptions(previousDisplayControlValue);
	lcdScreenDriver_setDisplayControlOptions(previousDisplayControlValue);

	uint8_t expectedNewDisplayControlValue = previousDisplayControlValue & ~(1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	helper_prepareSetDisplayControlOptions(expectedNewDisplayControlValue);

	lcdScreenDriver_setBlinkOff();
}

void test_lcdScreenDriver_setBlinkOn(void){
	uint8_t previousDisplayControlValue = (1 << LCDSCREEN_CONTROL_CURSOR_ON_BIT);
	helper_prepareSetDisplayControlOptions(previousDisplayControlValue);
	lcdScreenDriver_setDisplayControlOptions(previousDisplayControlValue);

	uint8_t expectedNewDisplayControlValue = previousDisplayControlValue | (1 << LCDSCREEN_CONTROL_BLINK_ON_BIT);
	helper_prepareSetDisplayControlOptions(expectedNewDisplayControlValue);

	lcdScreenDriver_setBlinkOn();
}

void test_lcdScreenDriver_setTextFlowToLeftToRight(void){
	uint8_t previousDisplayModeOptions = (1 << LCDSCREEN_MODE_SHIFTINCREMENT_BIT);
	helper_prepareSetDisplayModeOptions(previousDisplayModeOptions);
	lcdScreenDriver_setDisplayMode(previousDisplayModeOptions);

	uint8_t textflowDirection = LCDSCREEN_TEXTFLOW_LEFTTORIGHT;
	uint8_t expectedDisplayModeOptions = previousDisplayModeOptions | (1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	helper_prepareSetDisplayModeOptions(expectedDisplayModeOptions);

	lcdScreenDriver_setTextFlow(textflowDirection);
}

void test_lcdScreenDriver_setTextFlowToRightToLeft(void){
	uint8_t previousDisplayModeOptions = (1 << LCDSCREEN_MODE_SHIFTINCREMENT_BIT) | (1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	helper_prepareSetDisplayModeOptions(previousDisplayModeOptions);
	lcdScreenDriver_setDisplayMode(previousDisplayModeOptions);

	uint8_t textflowDirection = LCDSCREEN_TEXTFLOW_RIGHTTOLEFT;
	uint8_t expectedDisplayModeOptions = previousDisplayModeOptions & ~(1 << LCDSCREEN_MODE_READ_LEFTTORIGHT_BIT);
	helper_prepareSetDisplayModeOptions(expectedDisplayModeOptions);

	lcdScreenDriver_setTextFlow(textflowDirection);
}

void test_lcdScreenDriver_printCharPrintsAtTheCurrentPosition(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t cursorColumn = 2;
	uint8_t cursorRow = 1;
	helper_prepareSetCursorPosition(lcdScreenI2CAddress, cursorColumn, cursorRow);
	lcdScreenDriver_setCursorPosition(cursorColumn, cursorRow);
	char myChar = 'C';
	helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);

	lcdScreenDriver_printChar(myChar);
}

void test_lcdScreenDriver_printCharWrapsToNextRowWhenAtEndOfColumn(void){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareSetCursorToHome();
	lcdScreenDriver_setCursorHome();

	uint8_t numberOfColumns = 16; //goes from 0-15
	char myChar = 'D';
	
	// prepare multiple writes to fill a row
	for(uint8_t i = 0; i < numberOfColumns; i++){
		helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);
	}

	//on the 17th write it should move the cursor first to the beginning of the second row
	helper_prepareSetCursorPosition(lcdScreenI2CAddress, 0, 1);
	helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);
	for(uint8_t i = 0; i < 17; i++){
		lcdScreenDriver_printChar(myChar);
	}
}

void test_lcdScreenDriver_printCharAtEndOfSecondRowWillWrapBackToFirstLine(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t cursorPositionColumn = 15;
	uint8_t cursorPositionRow = 1;
	helper_prepareSetCursorPosition(lcdScreenI2CAddress, cursorPositionColumn,cursorPositionRow);
	lcdScreenDriver_setCursorPosition(cursorPositionColumn, cursorPositionRow);
	char myChar = 'Y';
	//on writing on the 33nd character it should wrap back to home
	helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);
	helper_prepareSetCursorPosition(lcdScreenI2CAddress, 0, 0);
	helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);
	lcdScreenDriver_printChar(myChar);
	lcdScreenDriver_printChar(myChar);
}

void test_lcdScreenDriver_printCharWrapsToNextLineOnBackslashN(void){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareSetCursorToHome();
	lcdScreenDriver_setCursorHome();
	char myChar = 'Z';
	char myNewLine = '\n';
	//write a char
	helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);
	//writing a newline only moves cursor to the beginning of the next row
	helper_prepareSetCursorPosition(lcdScreenI2CAddress, 0, 1);
	//Continue on next row
	helper_prepareWriteDataByte(lcdScreenI2CAddress, myChar);

	lcdScreenDriver_printChar(myChar);
	lcdScreenDriver_printChar(myNewLine);
	lcdScreenDriver_printChar(myChar);
}

void test_lcdScreenDriver_printStringThatFitsARow(void){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareSetCursorToHome();
	lcdScreenDriver_setCursorHome();
	char someString[] = "hello";
	char* pointerToString = someString;
	char currentChar;
	while(currentChar = *(pointerToString++)){
		helper_prepareWriteDataByte(lcdScreenI2CAddress, currentChar);
	}
	//expects a nullterminated string
	lcdScreenDriver_printString(someString);
}

// void test_lcdScreenDriver_printStringExpectsWrapOver(void){
// 	uint8_t lcdScreenI2CAddress = 23;
// 	helper_prepareSetCursorToHome();
// 	lcdScreenDriver_setCursorHome();
// 	char longerString[] = "abcdefghijklmnopqrstuvwxyz"; //this hopefully are 26 characters
// 	char* pointerToString = longerString;
// 	for(uint8_t i = 0; i < 16; i++){
// 		helper_prepareWriteDataByte(lcdScreenI2CAddress, *pointerToString++);
// 	}
// 	helper_prepareSetCursorPosition(lcdScreenI2CAddress, 0, 1);
// 	char currentChar;
// 	while((currentChar = *(pointerToString++))){
// 		helper_prepareWriteDataByte(lcdScreenI2CAddress, currentChar);
// 	}
// 	lcdScreenDriver_printString(longerString);
// }

//#####################################################
//test helper functions
//#####################################################

void helper_ignoreI2CInteraction(void){
	i2c_setSlaveAddress_Ignore();
	i2c_sendStartCondition_IgnoreAndReturn(I2C_CODES_NO_ERROR);
	i2c_writeBytes_IgnoreAndReturn(I2C_CODES_NO_ERROR);
	i2c_sendStopCondition_Ignore();
}

void helper_prepareWriteIncludingBacklight(uint8_t lcdScreenI2CAddress, uint8_t data, uint8_t backlightState){
	i2c_setSlaveAddress_Expect(lcdScreenI2CAddress);
	i2c_sendStartCondition_ExpectAndReturn(I2C_CODES_NO_ERROR);
	uint8_t expectedDataToPassToI2C = data | backlightState;
	i2c_write_ExpectAndReturn(expectedDataToPassToI2C, I2C_CODES_NO_ERROR);
	i2c_sendStopCondition_Expect();
}

void helper_prepareWriteEnablePulse(uint8_t lcdScreenI2CAddress, uint8_t data){
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, data | LCDSCREEN_PULSE_ENABLE_BIT, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	delayAbstraction_delayMicroseconds_Expect(1); // enable pulse must be >450ns
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, data & ~LCDSCREEN_PULSE_ENABLE_BIT, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	delayAbstraction_delayMicroseconds_Expect(50); // commands need > 37us to settle
}

void helper_prepareWriteNibble(uint8_t lcdScreenI2CAddress, uint8_t data, uint8_t sendingMode){
	helper_prepareWriteIncludingBacklight(lcdScreenI2CAddress, data << 4 | sendingMode, LCDSCREEN_INTERNAL_BACKLIGHT_ON);
	helper_prepareWriteEnablePulse(lcdScreenI2CAddress, (data << 4) | sendingMode);
}

void helper_prepareWriteCommandByte(uint8_t lcdScreenI2CAddress, uint8_t data){
	uint8_t highNibble = data >> 4;
	uint8_t lowNibble = data & 0x0F;
	helper_prepareWriteNibble(lcdScreenI2CAddress, highNibble, LCDSCREEN_SENDING_MODE_COMMAND);
	helper_prepareWriteNibble(lcdScreenI2CAddress, lowNibble, LCDSCREEN_SENDING_MODE_COMMAND);
}

void helper_prepareSetDisplayControlOptions(uint8_t controlOptions){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SETDISPLAYCONTROL | controlOptions);
}

void helper_prepareSetScreenFunctionOptions(uint8_t functionOptions){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_FUNCTIONALITY_COMMAND | functionOptions);
}

void helper_prepareClearDisplay(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t clearDisplayCommand = LCDSCREEN_COMMAND_CLEAR_DISPLAY;
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, clearDisplayCommand);
	delayAbstraction_delayMilliseconds_Expect(2);
}

void helper_prepareSetDisplayModeOptions(uint8_t modeOptions){
	uint8_t lcdScreenI2CAddress = 23;
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, modeOptions| LCDSCREEN_COMMAND_SETMODE);
}

void helper_prepareSetCursorToHome(void){
	uint8_t lcdScreenI2CAddress = 23;
	uint8_t commandHome = LCDSCREEN_COMMAND_MOVE_CURSOR_HOME;
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, commandHome);
	delayAbstraction_delayMilliseconds_Expect(2);
}

void helper_prepareWriteDataByte(uint8_t lcdScreenI2CAddress, uint8_t data){
	uint8_t highNibble = data >> 4;
	uint8_t lowNibble = data & 0x0F;
	helper_prepareWriteNibble(lcdScreenI2CAddress, highNibble, LCDSCREEN_SENDING_MODE_DATA);
	helper_prepareWriteNibble(lcdScreenI2CAddress, lowNibble, LCDSCREEN_SENDING_MODE_DATA);
}

void helper_prepareSetCursorPosition(uint8_t lcdScreenI2CAddress, uint8_t cursorPositionColumn, uint8_t cursorPositionRow){
	uint16_t rowLcdOffsets[] = {0x00, 0x40}; //, 0x14, 0x54}; //From the original implementation.
	helper_prepareWriteCommandByte(lcdScreenI2CAddress, LCDSCREEN_COMMAND_SET_DDRAM_ADDR | cursorPositionColumn + rowLcdOffsets[cursorPositionRow]);

}

// /* DO THESE EVENTUALLY
// // These commands scroll the display without changing the RAM
// void LiquidCrystal_I2C::scrollDisplayLeft(void) {
// 	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
// }
// void LiquidCrystal_I2C::scrollDisplayRight(void) {
// 	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
// }

// // This will 'right justify' text from the cursor
// void LiquidCrystal_I2C::autoscroll(void) {
// 	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
// 	command(LCD_ENTRYMODESET | _displaymode);
// }

// // This will 'left justify' text from the cursor
// void LiquidCrystal_I2C::noAutoscroll(void) {
// 	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
// 	command(LCD_ENTRYMODESET | _displaymode);
// }

// // Allows us to fill the first 8 CGRAM locations
// // with custom characters
// void LiquidCrystal_I2C::createChar(uint8_t location, uint8_t charmap[]) {
// 	location &= 0x7; // we only have 8 locations 0-7
// 	command(LCD_SETCGRAMADDR | (location << 3));
// 	for (int i=0; i<8; i++) {
// 		write(charmap[i]);
// 	}
// }
// */