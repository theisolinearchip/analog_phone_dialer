
#define		DIALER_IDLE_TICKS			10 	// 100 ms since the timer1 is set each 10ms
#define 	DIALER_DEBOUNCING_TICKS 	2 	// same as ^

volatile int dialer_current_ticks = 0;

int dialer_last_change_ticks = 0;

unsigned char dialer_number = 0;
unsigned char dialer_number_ready = 0;

int dialer_current_state = 0;
int dialer_last_state = 0;
int dialer_last_state_debounced = 0;

void dialer_prepare_timer1() {
	TCCR1B |= (1 << WGM12); // CTC mode
	OCR1A = 78; // 7813; // compare match value (8Mhz with 1024 preescaler -> 7812.5Hz -> count to 7813 for a second) // 78 for each 10ms
	TIMSK1 |= (1 << OCIE1A); // enable compare match interrupt
}

void dialer_start_timer1() {
	TCNT1 = 0; // clear previous value (just in case)
	TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 preescaler
}

void dialer_stop_timer1() {
	TCCR1B &= ~((1 << CS12) | (1 << CS10));
}

void dialer_init() {	
	// OPERATION_DDR |= (1 << OPERATION_LED); // init on main

	//OPERATION_DDR &= ~(1 << OPERATION_DIALER); // input by default, no need to do that
	OPERATION_PORT |= (1 << OPERATION_DIALER); // BUT we need to activate the pull up

	dialer_prepare_timer1();
}

// returns -1 if there's no number available
char dialer_check_number_ready() {

	char n = -1;

	dialer_current_state = (OPERATION_PIN & (1 << OPERATION_DIALER)); // 1 -> no connection (pulse), so high (pull-up); 0 -> connected (baseline), so grounded

	if (dialer_current_ticks - dialer_last_change_ticks >= DIALER_IDLE_TICKS) {

		if (dialer_number_ready) {
			n = (dialer_number % 10); // since 0 is actually 10

			dialer_number_ready = 0;
			dialer_number = 0;
		}

		dialer_current_ticks = 0;
		dialer_last_change_ticks = 0;		
	}

	if (dialer_current_state != dialer_last_state) {
		dialer_last_change_ticks = dialer_current_ticks;
	}

	if (dialer_current_ticks - dialer_last_change_ticks >= DIALER_DEBOUNCING_TICKS) {

		if (dialer_current_state != dialer_last_state_debounced) {
			dialer_number_ready = 1;
			dialer_last_state_debounced = dialer_current_state;
			if (dialer_last_state_debounced) {
				dialer_number++; // only if pulse
				OPERATION_PORT |= (1 << OPERATION_LED);
			} else {
				OPERATION_PORT &= ~(1 << OPERATION_LED);
			}
		}
	}

	dialer_last_state = dialer_current_state;

	return n;
}