#define p0 1
#define p1 (1 << 1)
#define p2 (1 << 2)
#define p3 (1 << 3)
#define DELAY 500
// #define F_CPU 12000000
//#include <asf.h>
// #define __AVR_AT90S8515__

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "uart2/uart.h"

extern void output_grb(uint8_t * ptr, uint16_t count);

void print_byte(uint16_t val) {
	uint8_t i;
	for (i = 16; i > 0; i--) {
		PORTC |= 0x2;
		if (val&(1<<(i-1))) _delay_ms(1000);
		else _delay_ms(100);
		PORTC &= ~0x2;
		_delay_ms(500);
	}
	_delay_ms(3000);
}


int main (void) {
	unsigned int data_in;
	DDRC = 0xff;		/* make PORTC as output port */
	uart_init(UART_BAUD_SELECT(9600, 12000000L));	/* initialize USART with 9600 baud rate */
	PORTC = 0;
	//Setup for led buffer
	// uint8_t buf3[3*4] = {0,0,0,0,0,128,0,0,128,128,0,0};
	// DDRC = 0x3;
	// PORTC = 0;
	// uint8_t dir = 0;
	
	// Toggle LEDs on all PORTS
	while (1) {
		data_in = uart_getc();	/* receive data from Bluetooth device*/
		if (data_in != UART_NO_DATA) {
			PORTC |= 0x2;
			_delay_ms(500);
			PORTC &= ~0x2;
			_delay_ms(500);	
			print_byte(data_in);
		}
		
		if (data_in =='1') {
			PORTC |= 1;	/* Turn ON LED */
			uart_puts("LED_ON");/* send status of LED i.e. LED ON */
		} else if (data_in =='2') {
			PORTC &= ~1;	/* Turn OFF LED */
			uart_puts("LED_OFF"); /* send status of LED i.e. LED OFF */
		} else {
			uart_puts("Select proper option"); /* send message for selecting proper option */
		}
		// Send to leds - uses PORTC bit 0
		/*
		output_grb(buf3, (uint16_t)12);
		_delay_ms(10);
		if (buf3[0] == 255) dir = 1;
		else if (buf3[0] == 0) dir = 0;
		if (dir) buf3[0] -= 5;
		else buf3[0] += 5;
		*/
	}
	
	return 0;
}