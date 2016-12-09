// system include
#include "F28x_Project.h"

// local include
#include "setup.h"

// function declarations
__interrupt void cpu_timer0_isr(void);

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

    // configure CPU-Timer 0 interrupt every microsecond:
    GPIO_SetupPinMux(TIMER_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(TIMER_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    EALLOW;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;
    InitCpuTimers();   // For this example, only initialize the Cpu Timers
    ConfigCpuTimer(&CpuTimer0, 200, 1);
    CpuTimer0Regs.TCR.all = 0x4000;
    IER |= M_INT1;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    //data
    Uint16 resultsIndex = 0;
    Uint16 TimeResults[RESULTS_BUFFER_SIZE];

	// main loop
	for(;;){

    	// reset index if end has been reached
    	if(resultsIndex == RESULTS_BUFFER_SIZE)
    		resultsIndex = 0;

        TimeResults[resultsIndex] = CpuTimer0.InterruptCount;
        //DELAY_US(100);
        //AdcaResults[resultsIndex] = sampleADCA();
        //AdcbResults[resultsIndex] = sampleADCB();
    	resultsIndex++;

	}
}

// timer 0 clock
__interrupt void cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;

   // acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
