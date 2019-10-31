#include <stdio.h>
#include "Debug_uart.h"

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
void integration_runTempTester(void);

void startUart(void){
	stdout = stdin = &uart_str;
	uart_init();
	printf("started UART\n");
	printf("\n\n########Startup complete########\n");
}

int main( int argc, const char* argv[] ){
	startUart();
	printf("Hello world!\n");
}

