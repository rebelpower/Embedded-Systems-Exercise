#include <avr/io.h>

#include "i2cInterface.h"
#include "lcdScreenDriver.h"
#include "registerAbstraction.h"
#include "delayAbstraction.h"

#define I2C_BAUDRATEREGISTER TWBR
#define I2C_STATUSREGISTER TWSR
#define I2C_CONTROLREGISTER TWCR
#define I2C_DATAREGISTER TWDR
#define I2C_PULLUPPINS_DIRECTIONREGISTER DDRC
#define I2C_PULLUPPINS_PORTREGISTER PORTC
#define I2C_SDA_PIN PC4 
#define I2C_SCL_PIN PC5

I2C_Registers myI2CRegisters = {
	&I2C_BAUDRATEREGISTER, 
	&I2C_STATUSREGISTER, 
	&I2C_CONTROLREGISTER, 
	&I2C_DATAREGISTER,
	&I2C_PULLUPPINS_DIRECTIONREGISTER,
	&I2C_PULLUPPINS_PORTREGISTER,
	I2C_SDA_PIN,
	I2C_SCL_PIN
};

#define SCREEN_ADDRESS 0x27
#define NUMBER_OF_COLUMNS 16
#define NUMBER_OF_ROWS 2

//An integration test example of the LCD screen library.
//The expected implementation has to work with this lcd screen library.

int main(int argc, char const *argv[]){
	lcdScreenDriver_initialise(&myI2CRegisters, SCREEN_ADDRESS, NUMBER_OF_COLUMNS, NUMBER_OF_ROWS, LCDSCREEN_TYPE_5x8DOTS);
	lcdScreenDriver_initialiseScreenToKnownState();
	lcdScreenDriver_setBacklightOn();
	lcdScreenDriver_setCursorOn();
	lcdScreenDriver_clearDisplay();
	lcdScreenDriver_setCursorHome();

	char myString[] = "Hello Embedded\nSystems!";
	lcdScreenDriver_printString(myString);
	delayAbstraction_delayMilliseconds(1000);
	char currentChar = 'A';
	while(1){
		lcdScreenDriver_printChar(currentChar++);
		if(currentChar>'Z'){
			currentChar = 'A';
		}
		delayAbstraction_delayMilliseconds(2000);
	}
}