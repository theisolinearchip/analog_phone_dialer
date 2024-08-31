#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include <stdlib.h>

#define 	DTMF_KEY_OFF			'.'

#define 	DTMF_KEY_1				'1'
#define 	DTMF_KEY_2				'2'
#define 	DTMF_KEY_3				'3'
#define 	DTMF_KEY_A 				'A'

#define 	DTMF_KEY_4				'4'
#define 	DTMF_KEY_5				'5'
#define 	DTMF_KEY_6				'6'
#define 	DTMF_KEY_B				'B'

#define 	DTMF_KEY_7				'7'
#define 	DTMF_KEY_8				'8'
#define 	DTMF_KEY_9				'9'
#define 	DTMF_KEY_C				'C'

#define		DTMF_KEY_AST 			'*'
#define 	DTMF_KEY_0				'0'
#define 	DTMF_KEY_HASH 			'#'
#define 	DTMF_KEY_D 				'D'

/*
				1209 Hz 	1336 Hz 	1477 Hz 	1633 Hz
	697 Hz		1			2			3 			A
	770 Hz		4			5			6 			B
	852 Hz		7			8			9 			C
	941 Hz		*			0			# 			D
*/

// ROWS
#define 	DTMF_INCREMENT_697		28		//
#define 	DTMF_INCREMENT_770	 	31		//
#define 	DTMF_INCREMENT_852	 	34		//	
#define 	DTMF_INCREMENT_941	 	38		//	941 * 400 / 10416.667

// COLUMNS
#define 	DTMF_INCREMENT_1209		48		//  	
#define 	DTMF_INCREMENT_1336		54		// 	
#define 	DTMF_INCREMENT_1477		60		//  	
#define 	DTMF_INCREMENT_1633		64		// 	(not tested IRL)

#define 	DTMF_TABLE_L			400		//


volatile unsigned int dtmf_index_low = 0;
volatile unsigned int dtmf_index_high = 0;

volatile unsigned int dtmf_selected_increment_low = 0;
volatile unsigned int dtmf_selected_increment_high = 0;

// using wavetable synthesis (https://thewolfsound.com/sound-synthesis/wavetable-synthesis-algorithm/)
// arduino reference library (not exactly working with those params, at least on my own tests: https://github.com/dilshan/dtmfgen)

// 400 samples (generated from https://www.daycounter.com/Calculators/Sine-Generator-Calculator.phtml)
const unsigned char sinewave_table[DTMF_TABLE_L] = {
	64,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,91,92,93,94,95,96,97,98,98,99,100,101,102,102,103,104,105,105,106,107,108,
	108,109,110,110,111,112,112,113,114,114,115,115,116,117,117,118,118,119,119,120,120,121,121,121,122,122,123,123,123,124,124,124,124,125,125,125,125,126,126,126,126,126,126,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,126,126,126,126,126,126,125,125,125,125,124,124,124,124,123,123,123,122,122,121,121,121,120,120,119,119,118,118,117,117,116,115,115,114,114,113,112,112,111,110,110,109,
	108,108,107,106,105,105,104,103,102,102,101,100,99,98,98,97,96,95,94,93,92,91,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,
	64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,36,35,34,33,32,31,30,29,29,28,27,26,25,25,24,23,22,22,21,20,19,
	19,18,17,17,16,15,15,14,13,13,12,12,11,10,10,9,9,8,8,7,7,6,6,6,5,5,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,6,7,7,8,8,9,9,10,10,11,12,12,13,13,14,15,15,16,17,17,18,
	19,19,20,21,22,22,23,24,25,25,26,27,28,29,29,30,31,32,33,34,35,36,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
};

void dtmf_prepare_timer0() {
	TCCR0A |= (1 << WGM01); // CTC mode
	OCR0A = 12; // compare match value 
	TIMSK0 |= (1 << OCIE0A); // enable compare match interrupt
}

void dtmf_start_timer0() {
	TCNT0 = 0; // clear previous value (just in case)
	TCCR0B |= (1 << CS01) | (1 << CS00); // 64 preescaler -> 125 kHz, count to 12 (OCR0A) to have 10416.667 ~= 10kHz
}

void dtmf_stop_timer0() {
	TCCR0B &= ~((1 << CS01) | (1 << CS00));
}

void dtmf_init() {
	DAC_DDR = 0xFF; // all port output
	DAC_PORT = 0;

	dtmf_prepare_timer0();
}

void dtmf_set_key(unsigned char key, unsigned int delay) {

	// led on (?)
	OPERATION_PORT |= (1 << OPERATION_LED);

	switch(key) {
		default:
		case DTMF_KEY_OFF:
			// led off
			OPERATION_PORT &= ~(1 << OPERATION_LED);
			return;
		case DTMF_KEY_1:
			dtmf_selected_increment_low = DTMF_INCREMENT_697;
			dtmf_selected_increment_high = DTMF_INCREMENT_1209;
			break;
		case DTMF_KEY_2:
			dtmf_selected_increment_low = DTMF_INCREMENT_697;
			dtmf_selected_increment_high = DTMF_INCREMENT_1336;
			break;
		case DTMF_KEY_3:
			dtmf_selected_increment_low = DTMF_INCREMENT_697;
			dtmf_selected_increment_high = DTMF_INCREMENT_1477;
			break;
		case DTMF_KEY_4:
			dtmf_selected_increment_low = DTMF_INCREMENT_770;
			dtmf_selected_increment_high = DTMF_INCREMENT_1209;
			break;
		case DTMF_KEY_5:
			dtmf_selected_increment_low = DTMF_INCREMENT_770;
			dtmf_selected_increment_high = DTMF_INCREMENT_1336;
			break;
		case DTMF_KEY_6:
			dtmf_selected_increment_low = DTMF_INCREMENT_770;
			dtmf_selected_increment_high = DTMF_INCREMENT_1477;
			break;
		case DTMF_KEY_7:
			dtmf_selected_increment_low = DTMF_INCREMENT_852;
			dtmf_selected_increment_high = DTMF_INCREMENT_1209;
			break;
		case DTMF_KEY_8:
			dtmf_selected_increment_low = DTMF_INCREMENT_852;
			dtmf_selected_increment_high = DTMF_INCREMENT_1336;
			break;
		case DTMF_KEY_9:
			dtmf_selected_increment_low = DTMF_INCREMENT_852;
			dtmf_selected_increment_high = DTMF_INCREMENT_1477;
			break;
		case DTMF_KEY_0:
			dtmf_selected_increment_low = DTMF_INCREMENT_941;
			dtmf_selected_increment_high = DTMF_INCREMENT_1336;
			break;
		case DTMF_KEY_AST: // not tested
			dtmf_selected_increment_low = DTMF_INCREMENT_941;
			dtmf_selected_increment_high = DTMF_INCREMENT_1209;
			break;
		case DTMF_KEY_HASH: // not tested
			dtmf_selected_increment_low = DTMF_INCREMENT_941;
			dtmf_selected_increment_high = DTMF_INCREMENT_1477;
			break;
	}

	dtmf_index_low = 0;
	dtmf_index_high = 0;

	dtmf_start_timer0();

	// block until the signal is transmited
	unsigned int a = 0;
	while(a < delay) {
		a++;
		_delay_ms(1);
	}

	dtmf_stop_timer0();

	DAC_PORT = sinewave_table[0] + sinewave_table[0];

	// led off
	OPERATION_PORT &= ~(1 << OPERATION_LED);
}

void dtmf_dial_number(int l, char *b) {

	for (int i = 0; i < l; i++) {
		dtmf_set_key(b[i], 200);

		unsigned int a = 0;
		while(a < 200) {
			a++;
			_delay_ms(1);
		}
	}

}