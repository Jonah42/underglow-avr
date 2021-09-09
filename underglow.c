// #define p0 1
// #define p1 (1 << 1)
// #define p2 (1 << 2)
// #define p3 (1 << 3)
// #define DELAY 500
#define F_CPU 12000000UL
#define NUM_LEDS 4
//#include <asf.h>
// #define __AVR_AT90S8515__

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "uart2/uart.h"

extern void output_grb(uint8_t * ptr, uint16_t count);
uint8_t curr_r = 0;
uint8_t curr_g = 0;
uint8_t curr_b = 0;
uint8_t orig_r = 0;
uint8_t orig_g = 0;
uint8_t orig_b = 0;
bool pulse_dir = 0;
bool cancel = false;

void setColour(uint8_t r, uint8_t g, uint8_t b) {
	uint8_t * buf = malloc(NUM_LEDS*3);
	for (int i = 0; i < NUM_LEDS; i++) {
		buf[i*3] = g;
		buf[i*3+1] = r;
		buf[i*3+2] = b;
	}
	output_grb(buf, (uint16_t)NUM_LEDS*3);
	free(buf);
}

void setColourPulse(uint8_t r, uint8_t g, uint8_t b) {
	curr_r = r;
	curr_g = g;
	curr_b = b;
	uint8_t * buf = malloc(NUM_LEDS*3);
	for (int i = 0; i < NUM_LEDS; i++) {
		buf[i*3] = g;
		buf[i*3+1] = r;
		buf[i*3+2] = b;
	}
	output_grb(buf, (uint16_t)NUM_LEDS*3);
	free(buf);
	TCNT0 += 120;
	
}

ISR (TIMER0_OVF_vect) {
	if (!pulse_dir) {
		curr_r = (curr_r < 5) ? 0 : curr_r - 5;
		curr_g = (curr_g < 5) ? 0 : curr_g - 5;
		curr_b = (curr_b < 5) ? 0 : curr_b - 5;
		if (!curr_r && !curr_g && !curr_b) pulse_dir = 1;
	} else {
		curr_r = (orig_r-curr_r < 5) ? orig_r : curr_r + 5;
		curr_g = (orig_g-curr_g < 5) ? orig_g : curr_g + 5;
		curr_b = (orig_b-curr_b < 5) ? orig_b : curr_b + 5;
		if (curr_r == orig_r && curr_g == orig_g && curr_b == orig_b) pulse_dir = 0;
	}
	// setColour(0,0,0);
	if (!cancel) setColourPulse(curr_r, curr_g, curr_b);
	else {
		cancel = false;
		TCCR0 &= 0xF8;
	}
}

int main (void) {
	unsigned int data_in;
	DDRC = 0xff;		/* make PORTC as output port */
	DDRD = 0x2;	
	uart_init(UART_BAUD_SELECT(9600, F_CPU));	/* initialize USART with 9600 baud rate */
	uint8_t *buf = malloc(128);
	int buf_end = 0;
    sei();
    char red, green, blue;

	//Setup for led buffer
	// uint8_t buf3[3*4] = {0,0,0,0,0,128,0,0,128,128,0,0};
	DDRC = 0x3;
	PORTC = 0;
	// uint8_t dir = 0;
	TCNT0 = 120;
	TCCR0 = (TCCR0&0xF8)|0x5;
	TIMSK |= 1 << TOIE0;
	orig_r = 0;
	orig_g = 0;
	orig_b = 255;
	setColourPulse(0,0,255);
	// Toggle LEDs on all PORTS
	while (1) {
		data_in = uart_getc();	/* receive data from Bluetooth device*/
		if (data_in == UART_NO_DATA) {
			continue;
		}
		buf[buf_end] = (uint8_t) data_in;
		buf_end++;
		if (buf_end == 9) {
			char tmp[4];
			tmp[3] = '\0';
			tmp[0] = buf[0];
			tmp[1] = buf[1];
			tmp[2] = buf[2];
			red = atoi(tmp);
			tmp[0] = buf[3];
			tmp[1] = buf[4];
			tmp[2] = buf[5];
			green = atoi(tmp);
			tmp[0] = buf[6];
			tmp[1] = buf[7];
			tmp[2] = buf[8];
			blue = atoi(tmp);
			PORTC ^= 0x2;
			cancel = true;
			setColour(red,green,blue);
			// sei();
			buf_end = 0;
			uart_puts("ACK\n");
		}
		// if (data_in =='1') {
		// 	PORTC |= 1;	/* Turn ON LED */
		// 	uart_puts("LED_ON\n");/* send status of LED i.e. LED ON */
		// } else if (data_in =='2') {
		// 	PORTC &= ~1;	 Turn OFF LED 
		// 	uart_puts("LED_OFF\n"); /* send status of LED i.e. LED OFF */
		// } else {
		// 	uart_puts("Select proper option\n"); /* send message for selecting proper option */
		// }
		// Send to leds - uses PORTC bit 0
		
		// output_grb(buf3, (uint16_t)12);
		// _delay_ms(10);
		// if (buf3[0] == 255) dir = 1;
		// else if (buf3[0] == 0) dir = 0;
		// if (dir) buf3[0] -= 5;
		// else buf3[0] += 5;
		
	}
	
	return 0;
}