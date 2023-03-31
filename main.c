#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include <stdlib.h>

#include "constants.h"
#include "dialer.h"
#include "dtmf.h"

#define MAX_TICKS 				1000 // 10000 ms max time measured (after that, counter will remain on this max value 'til reset)

#define MODE_BUFFERING_TICKS 	400	// 4000 ms since the timer1 is set each 10ms
#define MODE_LINE_OPEN_TICKS 	400	// 2000 ms since the timer1 is set each 10ms

char phone_buffer[PHONE_BUFFER_LENGTH]; // = {'1','2','3','4','5','6','7','8','9'};
unsigned int current_phone_buffer_length = 0;

volatile unsigned char current_mode = MODE_PHONE_WAITING;

volatile int mode_buffering_ticks = 0;
volatile int mode_line_open_ticks = 0;

ISR(TIMER0_COMPA_vect) {

 	PORTB = sinewave_table[dtmf_index_low] + sinewave_table[dtmf_index_high];

	dtmf_index_low += dtmf_selected_increment_low;
	if (dtmf_index_low >= DTMF_TABLE_L) dtmf_index_low -= DTMF_TABLE_L;

	dtmf_index_high += dtmf_selected_increment_high;
	if (dtmf_index_high >= DTMF_TABLE_L) dtmf_index_high -= DTMF_TABLE_L;

}

ISR(TIMER1_COMPA_vect) {
	
	// dialer counter
	dialer_current_ticks++;

	// misc counter for use in different modes
	if (mode_buffering_ticks < MAX_TICKS) mode_buffering_ticks++;
	if (mode_line_open_ticks < MAX_TICKS) mode_line_open_ticks++;
	
}

inline void open_line() {
	OPERATION_PORT |= (1 << OPERATION_RELAY_1);
}

inline void close_line() {
	OPERATION_PORT &= ~(1 << OPERATION_RELAY_1); // close line
}

inline void reset_phone_buffer()	{
	for (int i = 0; i < PHONE_BUFFER_LENGTH; i++) {
		phone_buffer[i] = DTMF_KEY_OFF;
	}
	current_phone_buffer_length = 0;
}

inline int check_for_line_reset() {
	if (OPERATION_PIN & (1 << OPERATION_BUTTON_2) ) {
		close_line();
		current_mode = MODE_PHONE_WAITING;

		return 1;
	}
	return 0;
}

int main(void) {

	for (int i = 0; i < PHONE_BUFFER_LENGTH; i++) {
		phone_buffer[i] = DTMF_KEY_OFF;
	}

	// already on input
	OPERATION_PORT |= ((1 << OPERATION_BUTTON_1) | (1 << OPERATION_BUTTON_2)); // put them on pullup

	// outputs
	OPERATION_DDR |= (1 << OPERATION_LED) | (1 << OPERATION_RELAY_1);
	OPERATION_PORT &= ~((1 << OPERATION_LED) | (1 << OPERATION_RELAY_1));

	dialer_init();
	dtmf_init();

	sei();

	dialer_start_timer1();

	while(1) {

		switch(current_mode) {
			default:
				break;
			case MODE_PHONE_WAITING:
				// wait until fetching the headphone
				if (!(OPERATION_PIN & (1 << OPERATION_BUTTON_2)) ) { // phone switch on Normally Opened, so check for ground (it's released)
					mode_line_open_ticks = 0;
					reset_phone_buffer();

					current_mode = MODE_PHONE_LINE_OPEN;
					open_line();
				}

				break;
			case MODE_PHONE_LINE_OPEN:
				// line will remain opened until hanging the phone UNLESS there's a digit input on the first...
				// 3 seconds? in that case will hang again and switch to PHONE BUFFERING MODE to continue with the input

				if (mode_line_open_ticks < MODE_LINE_OPEN_TICKS) {
					char n = dialer_check_number_ready();

					if (n > -1) {
						close_line();

						phone_buffer[current_phone_buffer_length++] = (n + '0');
						mode_buffering_ticks = 0; // reset waiting time on each new number

						current_mode = MODE_PHONE_BUFFERING;
					}
				} else {
					// after the initial waiting for the first input, just listen and repeat every single number
					char n = dialer_check_number_ready();

					if (n > -1) {
						phone_buffer[0] = (n + '0');
						dtmf_dial_number(1, phone_buffer);
					}
				}

				check_for_line_reset();
				break;
			case MODE_PHONE_BUFFERING:
				// listen for a new "key", if so, reset the wait time for switching to next mode
				// (we already have an initial key from the previous mode)
				if (current_phone_buffer_length < PHONE_BUFFER_LENGTH) {
					char n = dialer_check_number_ready();

					if (n > -1) {
						phone_buffer[current_phone_buffer_length++] = (n + '0');
						mode_buffering_ticks = 0; // reset waiting time on each new number
					}
				}

				if (mode_buffering_ticks >= MODE_BUFFERING_TICKS) {
					current_mode = MODE_PHONE_DIALING;
				}

				check_for_line_reset();
				break;
			case MODE_PHONE_DIALING:
				open_line();
				
				// delay a little bit when opening the line
				_delay_ms(500);

				dtmf_dial_number(current_phone_buffer_length, phone_buffer);
				reset_phone_buffer(); // reset after dialing

				current_mode = MODE_PHONE_LINE_OPEN; // notice the mode_line_open_ticks is not reset until going back to WAITING mode

				check_for_line_reset();
				break;
		}

	}

}