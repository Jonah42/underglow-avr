// #define p0 1
// #define p1 (1 << 1)
// #define p2 (1 << 2)
// #define p3 (1 << 3)
// #define DELAY 500
#define F_CPU 12000000UL
#define NUM_LEDS 4

#define SOLID 0
#define PULSE 1
#define ROTATE 2
#define VOLUME 3
#define FREQUENCY 4
#define CUSTOM 5

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
uint8_t curr_b = 255;
uint8_t primary_r = 0;
uint8_t primary_g = 0;
uint8_t primary_b = 255;
uint8_t secondary_r = 0;
uint8_t secondary_g = 0;
uint8_t secondary_b = 0;
uint16_t pulse_period = 120;
bool pulse_dir = 0; //True towards primary, false towards secondary
bool cancel = false;
uint8_t mode = 0;
const uint16_t paste_map[256] PROGMEM = {20,20,20,21,21,21,22,22,23,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,33,33,34,34,35,36,36,37,38,38,39,40,41,41,42,43,44,44,45,46,47,48,49,50,50,51,52,53,54,55,56,57,58,59,61,62,63,64,65,66,67,69,70,71,73,74,75,77,78,79,81,82,84,85,87,89,90,92,93,95,97,99,100,102,104,106,108,110,112,114,116,118,120,123,125,127,129,132,134,137,139,142,144,147,149,152,155,158,161,164,167,170,173,176,179,182,186,189,192,196,199,203,207,211,214,218,222,226,230,235,239,243,248,252,257,261,266,271,276,281,286,291,297,302,307,313,319,325,330,336,343,349,355,362,368,375,382,389,396,403,410,418,425,433,441,449,457,465,474,482,491,500,509,518,528,537,547,557,567,578,588,599,610,621,632,643,655,667,679,691,704,717,730,743,757,770,784,799,813,828,843,858,874,890,906,922,939,956,973,991,1009,1027,1046,1065,1084,1104,1124,1145,1165,1187,1208,1230,1252,1275,1298,1322,1346,1370,1395,1420,1446,1473,1499,1527,1554,1582,1611,1640,1670,1701,1731,1763,1795,1827,1861,1894,1929,1964};
// uint16_t hello[200] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,};
uint16_t clock = 0;

void setSolidColour(void) {
	uint8_t * buf = malloc(NUM_LEDS*3);
	// uint8_t buf[12];
	// PORTC |=0x2;
	for (int i = 0; i < NUM_LEDS; i++) {
		buf[i*3] = primary_g;
		buf[i*3+1] = primary_r;
		buf[i*3+2] = primary_b;
	}
	output_grb(buf, (uint16_t)NUM_LEDS*3);
	free(buf);
}

void setPrimaryRGB(uint8_t* buf) {
	buf[9] = '\0';
	primary_b = atoi((char*)buf+6);
	buf[6] = '\0';
	primary_g = atoi((char*)buf+3);
	buf[3] = '\0';
	primary_r = atoi((char*)buf);
}

void setSecondaryRGB(uint8_t* buf) {
	buf[9] = '\0';
	secondary_b = atoi((char*)buf+6);
	buf[6] = '\0';
	secondary_g = atoi((char*)buf+3);
	buf[3] = '\0';
	secondary_r = atoi((char*)buf);
}

void setPulsePeriod(uint8_t* buf) {
	buf[3] = '\0';
	uint8_t pulse_scale = atoi((char*)buf);
	// pulse_period = pulse_scale < 128 ? paste_map_1[pulse_scale] : paste_map_2[pulse_scale-128];
	pulse_period = paste_map[pulse_scale];
	// pulse_period = 120;
	// setColourPulse();
	if (pulse_period > 255) PORTC |= 0x2;
}

void setColourPulse(void) {
	// PORTC |= 0x2;
	uint8_t * buf = malloc(NUM_LEDS*3);
	for (int i = 0; i < NUM_LEDS; i++) {
		buf[i*3] = curr_g;
		buf[i*3+1] = curr_r;
		buf[i*3+2] = curr_b;
	}
	output_grb(buf, (uint16_t)NUM_LEDS*3);
	free(buf);
	if (pulse_period > 255) TCNT0 = 255;
	else TCNT0 = pulse_period;
	
}

ISR (TIMER0_OVF_vect) {

	if (mode != PULSE) {
		TCCR0 &= 0xF8;
		return;
	}
	if (pulse_period > 255) clock += 255;
	if (pulse_period > 255 && clock < pulse_period){
		TCNT0 = pulse_period-clock < 255 ? pulse_period-clock : 255;
		return;
	}
	clock = 0;
	if (!pulse_dir) { // towards secondary
		if (curr_r < secondary_r) curr_r = (secondary_r-curr_r < 5) ? secondary_r : curr_r + 5;
		else curr_r = (curr_r-secondary_r < 5) ? secondary_r : curr_r - 5;
		if (curr_g < secondary_g) curr_g = (secondary_g-curr_g < 5) ? secondary_g : curr_g + 5;
		else curr_g = (curr_g-secondary_g < 5) ? secondary_g : curr_g - 5;
		if (curr_b < secondary_b) curr_b = (secondary_b-curr_b < 5) ? secondary_b : curr_b + 5;
		else curr_b = (curr_b-secondary_b < 5) ? secondary_b : curr_b - 5;
		if (curr_r == secondary_r && curr_g == secondary_g && curr_b == secondary_b) pulse_dir = 1;
	} else { // towards primary
		if (curr_r < primary_r) curr_r = (primary_r-curr_r < 5) ? primary_r : curr_r + 5;
		else curr_r = (curr_r-primary_r < 5) ? primary_r : curr_r - 5;
		if (curr_g < primary_g) curr_g = (primary_g-curr_g < 5) ? primary_g : curr_g + 5;
		else curr_g = (curr_g-primary_g < 5) ? primary_g : curr_g - 5;
		if (curr_b < primary_b) curr_b = (primary_b-curr_b < 5) ? primary_b : curr_b + 5;
		else curr_b = (curr_b-primary_b < 5) ? primary_b : curr_b - 5;
		if (curr_r == primary_r && curr_g == primary_g && curr_b == primary_b) pulse_dir = 0;
	}
	// setColour(0,0,0);
	// PORTC &= ~0x2;
	setColourPulse();
}

uint8_t determineExpected(uint8_t command) {
	uint8_t expected = 0;
	if (command < 10) expected = 9;
	else if (command < 20) {
		switch (command) {
			case 10: expected = 21; break;
			case 11: expected = 9; break;
			case 12: expected = 9; break;
			case 13: expected = 3; break;
		}
	}
	else if (command < 30) {
		switch (command) {
			case 20: expected = 24; break;
			case 21: expected = 18; break;
			case 22: expected = 3; break;
			case 23: expected = 3; break;
		}
	}
	else if (command < 40) expected = 9;
	else if (command < 50) expected = 9;
	else if (command < 60) expected = 9;
	return expected;
}

void performUpdate(uint8_t command, uint8_t expected, uint8_t* buf) {
	switch (command) {
		case 0:
			mode = SOLID;
			setPrimaryRGB(buf);
			setSolidColour();
			break;
		case 10:
			mode = PULSE;
			setPulsePeriod(buf+18);
			setSecondaryRGB(buf+9);
			setPrimaryRGB(buf);
			TCCR0 = (TCCR0&0xF8)|0x5;
			setColourPulse();
			break;
		case 11:
			setPrimaryRGB(buf);
			break;
		case 12:
			setSecondaryRGB(buf);
			break;
		case 13:
			setPulsePeriod(buf);
			break;
	}
}

int main (void) {
	unsigned int data_in;
	DDRC = 0xff;		/* make PORTC as output port */
	DDRD = 0x2;	
	uart_init(UART_BAUD_SELECT(9600, F_CPU));	/* initialize USART with 9600 baud rate */
	uint8_t *buf = malloc(128);
	uint8_t buf_end = 0;
    sei();
    // char red, green, blue;

	//Setup for led buffer
	// uint8_t buf3[3*4] = {0,0,0,0,0,128,0,0,128,128,0,0};
	// DDRC = 0x3;
	PORTC = 0;
	// uint8_t dir = 0;
	TCNT0 = pulse_period;
	
	TIMSK |= 1 << TOIE0;
	// mode = SOLID;
	// PORTC |= 0x2;
	// setSolidColour();
	// while(1){}
	mode = PULSE;
	// setPulsePeriod((uint8_t*)"120");
	// setSecondaryRGB((uint8_t*)"000000255");
	// setPrimaryRGB((uint8_t*)"000000000");
	TCCR0 = (TCCR0&0xF8)|0x5;
	setColourPulse();
	uint8_t command = 0;
	uint8_t expected;
	// Toggle LEDs on all PORTS
	while (1) {
		data_in = uart_getc();	/* receive data from Bluetooth device*/
		if (data_in == UART_NO_DATA) {
			continue;
		}
		buf[buf_end] = (uint8_t) data_in;
		buf_end++;
		if (buf_end == 2) { // Got command byte
			buf[2] = '\0';
			command = atoi((char*)buf);
			expected = determineExpected(command);
			buf_end = 0;
			while (buf_end < expected) {
				data_in = uart_getc();	/* receive data from Bluetooth device*/
				if (data_in == UART_NO_DATA) {
					continue;
				}
				buf[buf_end] = (uint8_t) data_in;
				buf_end++;
			}
			performUpdate(command, expected, buf);
			buf_end = 0;
			uart_puts("ACK\n");
		}
		/*if (buf_end == 9) {
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
		}*/
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