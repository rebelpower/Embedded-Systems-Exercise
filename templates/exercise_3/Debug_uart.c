/*! \file uart.c
 \brief Minimal UART library used for debugging and demo.
 */
#include "Debug_uart.h"

#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

#ifndef BAUD
#define BAUD 9600
#endif
#ifndef F_CPU
#define F_CPU 16000000
#endif

#define SCHEDULER_UBRR (F_CPU/16/BAUD-1)

void uart_init() {
	char cSREG = SREG;
	cli();

	UBRR0H = (unsigned char) (SCHEDULER_UBRR >> 8);
	UBRR0L = (unsigned char) SCHEDULER_UBRR;
	UCSR0B = ((1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0));		// Enable receiver and transmitter and Rx interrupt
	UCSR0C = ((0 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00));	// Set frame format: 8data, 1 stop bit. See Table 22-7 for details

	SREG = cSREG;
}

void uart_transmit(unsigned char data) {
	/* Wait for empty transmit buffer */
	while (!( UCSR0A & (1 << UDRE0)))
		;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
unsigned char uart_receive(void) {
	/* Wait for data to be received */
	while (!(UCSR0A & (1 << RXC0)))
		;
	/* Get and return received data from buffer */
	return UDR0;
}

//void uart_flush(void) {
//	unsigned char dummy;
//	dummy = 0;
//	while ( UCSR1A & (1 << RXC1)) {
//		dummy = UDR1;
//	}
//}

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int uart_putchar(char c, FILE *stream) {

	if (c == '\a') {
		fputs("*ring*\n", stderr);
		return 0;
	}

	if (c == '\n')
		uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;

	return 0;
}

int uart_printf(char var, FILE *stream) {
#ifdef PRINT
	if (var == '\n')
		uart_transmit('\r');
	uart_transmit(var);
#endif
	return 0;
}

/* only for the following part
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, UART implementation
 *
 * $Id: uart.c 1008 2005-12-28 21:38:59Z joerg_wunsch $
 */


 // * Receive a character from the UART Rx.
 // *
 // * This features a simple line-editor that allows to delete and
 // * re-edit the characters entered, until either CR or NL is entered.
 // * Printable characters entered will be echoed using uart_putchar().
 // *
 // * Editing characters:
 // *
 // * . \b (BS) or \177 (DEL) delete the previous character
 // * . ^u kills the entire input buffer
 // * . ^w deletes the previous word
 // * . ^r sends a CR, and then reprints the buffer
 // * . \t will be replaced by a single space
 // *
 // * All other control characters will be ignored.
 // *
 // * The internal line buffer is RX_BUFSIZE (80) characters long, which
 // * includes the terminating \n (but no terminating \0).  If the buffer
 // * is full (i. e., at RX_BUFSIZE-1 characters in order to keep space for
 // * the trailing \n), any further input attempts will send a \a to
 // * uart_putchar() (BEL character), although line editing is still
 // * allowed.
 // *
 // * Input errors while talking to the UART will cause an immediate
 // * return of -1 (error indication).  Notably, this will be caused by a
 // * framing error (e. g. serial line "break" condition), by an input
 // * overrun, and by a parity error (if parity was enabled and automatic
 // * parity recognition is supported by hardware).
 // *
 // * Successive calls to uart_getchar() will be satisfied from the
 // * internal buffer until that buffer is emptied again.
 

int uart_getchar(FILE *stream) {
	uint8_t c;
	char *cp, *cp2;
	static char b[RX_BUFSIZE];
	static char *rxp;

	if (rxp == 0)
		for (cp = b;;) {
			loop_until_bit_is_set(UCSR0A, RXC0);
			if (UCSR0A & _BV(FE0))
				return _FDEV_EOF;
			if (UCSR0A & _BV(DOR0))
				return _FDEV_ERR;
			c = UDR0;
			/* behaviour similar to Unix stty ICRNL */
			if (c == '\r')
				c = '\n';
			if (c == '\n') {
				*cp = c;
				uart_putchar(c, stream);
				rxp = b;
				break;
			} else if (c == '\t')
				c = ' ';

			if ((c >= (uint8_t) ' ' && c <= (uint8_t) '\x7e') || c >= (uint8_t) '\xa0') {
				if (cp == b + RX_BUFSIZE - 1)
					uart_putchar('\a', stream);
				else {
					*cp++ = c;
					uart_putchar(c, stream);
				}
				continue;
			}

			switch (c) {
			case 'c' & 0x1f:
				return -1;

			case '\b':
			case '\x7f':
				if (cp > b) {
					uart_putchar('\b', stream);
					uart_putchar(' ', stream);
					uart_putchar('\b', stream);
					cp--;
				}
				break;

			case 'r' & 0x1f:
				uart_putchar('\r', stream);
				for (cp2 = b; cp2 < cp; cp2++)
					uart_putchar(*cp2, stream);
				break;

			case 'u' & 0x1f:
				while (cp > b) {
					uart_putchar('\b', stream);
					uart_putchar(' ', stream);
					uart_putchar('\b', stream);
					cp--;
				}
				break;

			case 'w' & 0x1f:
				while (cp > b && cp[-1] != ' ') {
					uart_putchar('\b', stream);
					uart_putchar(' ', stream);
					uart_putchar('\b', stream);
					cp--;
				}
				break;
			}
		}

	c = *rxp++;
	if (c == '\n')
		rxp = 0;

	return c;
}
