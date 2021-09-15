// #define p0 1
// #define p1 (1 << 1)
// #define p2 (1 << 2)
// #define p3 (1 << 3)
// #define DELAY 500
#define F_CPU 12000000UL
#define NUM_LEDS 470UL

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
extern void output_grb_solid(uint8_t * ptr, uint16_t count);
extern void output_grb2(uint8_t * ptr, uint16_t count);
uint8_t tmp_buf[] = {0,0,255};
uint8_t * led_buf;
uint8_t curr_r = 0;
uint8_t curr_g = 0;
uint8_t curr_b = 255;
uint8_t primary_r = 0;
uint8_t primary_g = 0;
uint8_t primary_b = 255;
uint8_t secondary_r = 0;
uint8_t secondary_g = 0;
uint8_t secondary_b = 0;
uint16_t period = 120;
uint16_t rotate_width = 1;
bool pulse_dir = 0; //True towards primary, false towards secondary
bool cancel = false;
uint8_t mode = 0;
const uint16_t paste_map[] PROGMEM = {20,20,20,21,21,21,22,22,23,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31,32,33,33,34,34,35,36,36,37,38,38,39,40,41,41,42,43,44,44,45,46,47,48,49,50,50,51,52,53,54,55,56,57,58,59,61,62,63,64,65,66,67,69,70,71,73,74,75,77,78,79,81,82,84,85,87,89,90,92,93,95,97,99,100,102,104,106,108,110,112,114,116,118,120,123,125,127,129,132,134,137,139,142,144,147,149,152,155,158,161,164,167,170,173,176,179,182,186,189,192,196,199,203,207,211,214,218,222,226,230,235,239,243,248,252,257,261,266,271,276,281,286,291,297,302,307,313,319,325,330,336,343,349,355,362,368,375,382,389,396,403,410,418,425,433,441,449,457,465,474,482,491,500,509,518,528,537,547,557,567,578,588,599,610,621,632,643,655,667,679,691,704,717,730,743,757,770,784,799,813,828,843,858,874,890,906,922,939,956,973,991,1009,1027,1046,1065,1084,1104,1124,1145,1165,1187,1208,1230,1252,1275,1298,1322,1346,1370,1395,1420,1446,1473,1499,1527,1554,1582,1611,1640,1670,1701,1731,1763,1795,1827,1861,1894,1929,1964};
// uint16_t hello[200] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,};
uint16_t clock = 0;

void setSolidColour(void) {
	// uint8_t * tmp_buf = malloc(3);
	// for (uint16_t i = 0; i < NUM_LEDS; i++) {
		tmp_buf[0] = primary_g;
		tmp_buf[1] = primary_r;
		tmp_buf[2] = primary_b;
		// led_buf[0] = primary_g;
		// led_buf[1] = primary_r;
		// led_buf[2] = primary_b;

		// if (i == 119) PORTC |= 2;
	// }
	PORTC |= 2;
	output_grb_solid(tmp_buf, ((uint16_t)NUM_LEDS)*3);
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

void setPeriod(uint8_t* buf) {
	buf[3] = '\0';
	uint8_t pulse_scale = atoi((char*)buf);
	// pulse_period = pulse_scale < 128 ? paste_map_1[pulse_scale] : paste_map_2[pulse_scale-128];
	period = pgm_read_word(&paste_map[pulse_scale]);
	// pulse_period = 120;
	// setColourPulse();
	// if (pulse_period > 255) PORTC |= 0x2;
}

void setColourPulse(void) {
	// for (int i = 0; i < NUM_LEDS; i++) {
		tmp_buf[0] = curr_g;
		tmp_buf[1] = curr_r;
		tmp_buf[2] = curr_b;
	// }
	output_grb_solid(tmp_buf, (uint16_t)NUM_LEDS*3);
	if (period > 255) TCNT0 = 0;
	else TCNT0 = 255-period;
}

void setRotateWidth(uint8_t* buf) {
	buf[3] = '\0';
	uint16_t rotate_percent = atoi((char*)buf);
	rotate_width = NUM_LEDS*rotate_percent/100;
}

void setColourRotate(void) {
	static uint16_t rotate_peak = 0;
	for (int i = 0; i < NUM_LEDS; i++) {
		if ((i >= rotate_peak && i < rotate_peak+rotate_width) ||
			(rotate_peak+rotate_width > NUM_LEDS && i < rotate_peak+rotate_width-NUM_LEDS)) {
			led_buf[i*3] = primary_g;
			led_buf[i*3+1] = primary_r;
			led_buf[i*3+2] = primary_b;
		} else {
			led_buf[i*3] = secondary_g;
			led_buf[i*3+1] = secondary_r;
			led_buf[i*3+2] = secondary_b;
		}
	}
	rotate_peak++;
	if (rotate_peak == NUM_LEDS) rotate_peak = 0;
	output_grb(led_buf, (uint16_t)NUM_LEDS*3);
	if (period > 255) TCNT0 = 0;
	else TCNT0 = 255-period;
	// PORTC ^= 2;
}

uint8_t readADC(void) {
	// PORTC ^= 0x2;
	if (PINA > 0) PORTC |= 0x2;
	// if (PINA < 128) return 0;
	// return (PINA-128)<1;
	return PINA;
}

void setColourVolume(void) {
	static uint16_t prev = 0;
	// static uint16_t prev2 = 0;
	uint16_t volume = readADC();
	// // if (volume < 70) volume = 70;
	uint8_t t = 10;
	if (volume>prev && volume-prev > t) {
		volume = prev+t;
	} else if (volume<prev && prev-volume > t) {
		volume = prev-t;
	}
	PORTD &= ~0x4;
	curr_r = (primary_r*volume) >> 8;
	curr_g = (primary_g*volume) >> 8;
	curr_b = (primary_b*volume) >> 8;
	for (int i = 0; i < NUM_LEDS; i++) {
		led_buf[i*3] = curr_g;
		led_buf[i*3+1] = curr_r;
		led_buf[i*3+2] = curr_b;
	}
	output_grb(led_buf, (uint16_t)NUM_LEDS*3);
	PORTD ^= 0x4;
	// Refresh rate about 60Hz (probs calculated this wrong eek)
	TCNT0 = 120;
	// prev2 = prev;
	prev = volume;
}

void setColourFrequency(void) {
	uint8_t increment = 10;
	if (PIND&0x8) {
		if (curr_r < primary_r) curr_r = (primary_r-curr_r < increment) ? primary_r : curr_r + increment;
		else curr_r = (curr_r-primary_r < increment) ? primary_r : curr_r - increment;
		if (curr_g < primary_g) curr_g = (primary_g-curr_g < increment) ? primary_g : curr_g + increment;
		else curr_g = (curr_g-primary_g < increment) ? primary_g : curr_g - increment;
		if (curr_b < primary_b) curr_b = (primary_b-curr_b < increment) ? primary_b : curr_b + increment;
		else curr_b = (curr_b-primary_b < increment) ? primary_b : curr_b - increment;
	} else {
		if (curr_r < secondary_r) curr_r = (secondary_r-curr_r < increment) ? secondary_r : curr_r + increment;
		else curr_r = (curr_r-secondary_r < increment) ? secondary_r : curr_r - increment;
		if (curr_g < secondary_g) curr_g = (secondary_g-curr_g < increment) ? secondary_g : curr_g + increment;
		else curr_g = (curr_g-secondary_g < increment) ? secondary_g : curr_g - increment;
		if (curr_b < secondary_b) curr_b = (secondary_b-curr_b < increment) ? secondary_b : curr_b + increment;
		else curr_b = (curr_b-secondary_b < increment) ? secondary_b : curr_b - increment;
	}
	for (int i = 0; i < NUM_LEDS; i++) {
		led_buf[i*3] = curr_g;
		led_buf[i*3+1] = curr_r;
		led_buf[i*3+2] = curr_b;
	}
	output_grb(led_buf, (uint16_t)NUM_LEDS*3);
	// Refresh rate about 60Hz (probs calculated this wrong eek)
	TCNT0 = 250;
	// prev2 = prev;
}

ISR (TIMER0_OVF_vect) {
	if (mode != PULSE && mode != ROTATE && mode != VOLUME && mode != FREQUENCY) {
		TCCR0 &= 0xF8;
		return;
	}
	if (period > 255) clock += 255;
	if (period > 255 && clock < period) {
		TCNT0 = 255-(period-clock < 255 ? period-clock : 255);
		return;
	}
	clock = 0;
	if (mode == ROTATE) setColourRotate();
	if (mode == VOLUME) setColourVolume();
	if (mode == FREQUENCY) setColourFrequency();
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
			case 21: expected = 9; break;
			case 22: expected = 9; break;
			case 23: expected = 3; break;
			case 24: expected = 3; break;
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
			setPeriod(buf+18);
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
			setPeriod(buf);
			break;
		case 20:
			mode = ROTATE;
			setRotateWidth(buf+21);
			setPeriod(buf+18);
			setSecondaryRGB(buf+9);
			setPrimaryRGB(buf);
			TCCR0 = (TCCR0&0xF8)|0x5;
			setColourRotate();
			break;
		case 21:
			setPrimaryRGB(buf);
			break;
		case 22:
			setSecondaryRGB(buf);
			break;
		case 23:
			setPeriod(buf);
			break;
		case 24:
			setRotateWidth(buf);
			break;
		case 30:
			mode = VOLUME;
			setPrimaryRGB(buf);
			TCCR0 = (TCCR0&0xF8)|0x5;
			setColourVolume();
			break;
		case 40:
			mode = FREQUENCY;
			setPrimaryRGB(buf);
			secondary_r = primary_r >> 5;
			secondary_g = primary_g >> 5;
			secondary_b = primary_b >> 5;
			TCCR0 = (TCCR0&0xF8)|0x5;
			setColourFrequency();
	}
}

int main (void) {
	DDRA = 0;
	DDRC = 0xff;		/* make PORTC as output port */
	DDRD = 0x6;
	PORTD |= 0x4;
	uint16_t length = 4;
	led_buf = malloc(length*3);
	if (led_buf == NULL) PORTC |= 2;
	for (uint8_t i = 0; i < 4; i++) {
		led_buf[i*3] = 255;
		led_buf[i*3+1] = 0;
		led_buf[i*3+2] = 0;
	}
	unsigned int data_in;
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
	TCNT0 = period;
	
	TIMSK |= 1 << TOIE0;
	mode = SOLID;
	// PORTC |= 0x2;
	setSolidColour();
	// while(1){}
	// mode = VOLUME;
	// setPulsePeriod((uint8_t*)"120");
	// setSecondaryRGB((uint8_t*)"000000255");
	// setPrimaryRGB((uint8_t*)"000000000");
	// TCCR0 = (TCCR0&0xF8)|0x5;
	// period = 500;
	// setColourVolume();
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
			// if (expected == 24) PORTC |= 2;
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
			char str[20];
			sprintf(str, "ACK%d-%d\n", period, clock);
			// uart_puts("ACK\n");
			uart_puts(str);
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