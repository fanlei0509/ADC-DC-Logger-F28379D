// system include
#include "F28x_Project.h"

// local include
#include "setup.h"

// main execution code
void main(void) {

	// initialize setup
	setup_cpu2();

	// main loop
	for(;;){

        // turn on blinky
        GPIO_WritePin(BLINKY_LED_2, 0);

        // delay
        DELAY_US(1000*250);

        // turn off blinky
        GPIO_WritePin(BLINKY_LED_2, 1);

        // delay
        DELAY_US(1000*250);

	}
}
