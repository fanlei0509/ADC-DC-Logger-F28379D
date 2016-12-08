// system include
#include "F28x_Project.h"

// local include
#include "setup.h"

// main execution code
void main(void) {

	// initialize setup
	setup_cpu1();

	// main loop
	for(;;){
		DELAY_US(1000*500);
	}
}
