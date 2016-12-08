// system include
#include "F28x_Project.h"

// local include
#include "setup.h"

// defines


// main execution code
void main(void) {

	// initialize setup
	setup_cpu1();

	// setup blinky 1 and 2
    EALLOW;
    GPIO_SetupPinOptions(BLINKY_LED_1, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(BLINKY_LED_1, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLINKY_LED_2, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(BLINKY_LED_2, GPIO_MUX_CPU2, 0);
    EDIS;

	// main loop
	for(;;){

        // turn on blinky
        GPIO_WritePin(BLINKY_LED_1, 0);

        // delay
        DELAY_US(1000*500);

        // turn off blinky
        GPIO_WritePin(BLINKY_LED_1, 1);

        // delay
        DELAY_US(1000*500);

	}
}
