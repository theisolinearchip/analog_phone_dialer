
// ---

#define 	OPERATION_PORT				PORTC
#define 	OPERATION_DDR				DDRC
#define 	OPERATION_PIN				PINC

#define 	DAC_PORT					PORTB
#define 	DAC_DDR						DDRB

// ---

#define		OPERATION_LED 				PC0
#define		OPERATION_DIALER 			PC1
#define		OPERATION_BUTTON_1 			PC2		// button for UI
#define		OPERATION_RELAY_1 			PC3		// handles the open line (through the disc connectors switches)
#define		OPERATION_BUTTON_2			PC4		// internal swith for detecting phone up/down

// ---

#define 	PHONE_BUFFER_LENGTH 		50

#define		MODE_PHONE_WAITING			1 		// phone is down (no line enabled)
#define		MODE_PHONE_LINE_OPEN		2		// phone up, line open, buffer already outputed
#define 	MODE_PHONE_BUFFERING		3 		// phone up and reading the inputs from the dialer, line is closed
#define 	MODE_PHONE_DIALING			4 		// phone up and outputing the buffer with the dialed number, after that moves to line open