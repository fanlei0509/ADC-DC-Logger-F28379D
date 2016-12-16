// system include
#include "F28x_Project.h"
#include "ff.h"
#include "diskio.h"

// local include
#include "setup.h"

// function declarations
__interrupt void cpu_timer0_isr(void);
inline int16_t sampleADCA(void);
inline int16_t sampleADCB(void);
void setup_blinkys(void);
void setup_timer(void);
void setup_ADCs(void);
void twiddle_blinkys(void);

// variables
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */
#define RESULTS_BUFFER_SIZE 512
#define NUM_VALUES 8192  // size*3
#define BUFF_SIZE  16384 // size*3*2
Uint16 results[NUM_VALUES];
BYTE* data_pointer = (BYTE *)results;

// main execution code
void main(void) {

	// initialize setup
	setup_cpu1();

	// setup blinky 1 and 2
	setup_blinkys();

    // configure CPU-Timer 0 interrupt every microsecond:
	setup_timer();

    // setup ADCs
	setup_ADCs();

	// enable interrupts
    EINT;
    ERTM;

    //data
    Uint16 resultsIndex = 0;

    // setup data file
    f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    if(f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    	twiddle_blinkys();

    f_expand(&Fil, 6553600, 1);

    UINT bw;

    BYTE drv = Fil.obj.fs->drv;
    DWORD sect = Fil.obj.fs->database + Fil.obj.fs->csize * (Fil.obj.sclust - 2);

    /* Write 2048 sectors from top of the file at a time */


	// main loop
    Uint32 i;
    Uint32 start_time = CpuTimer0.InterruptCount;
	for(i = 0; i < 100; ++i){
		//f_write(&Fil, results, BUFF_SIZE, &bw);	/* Write data to the file */
	    disk_write(drv, data_pointer, sect, 32);
	    sect += 32;
	}
#if 0
	for(i = 0; i < RESULTS_BUFFER_SIZE*10; ++i){

    	results[resultsIndex] = CpuTimer0.InterruptCount16;
    	results[resultsIndex+1] = sampleADCA();
    	results[resultsIndex+2] = sampleADCB();

    	resultsIndex = resultsIndex + 3;

    	// reset index if end has been reached
    	if(resultsIndex == NUM_VALUES){
    		resultsIndex = 0;
    		f_write(&Fil, results, BUFF_SIZE, &bw);	/* Write data to the file */
    	}

	}
#endif
	Uint32 stop_time = CpuTimer0.InterruptCount;
	Uint32 dt = stop_time - start_time;
	DWORD sect2 = Fil.obj.fs->database + Fil.obj.fs->csize * (Fil.obj.sclust - 2);
	f_close(&Fil);


}

// timer 0 clock
__interrupt void cpu_timer0_isr(void)
{
	CpuTimer0.InterruptCount16++;
	CpuTimer0.InterruptCount++;


	// acknowledge this interrupt to receive more interrupts from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

// sample ADC-A
int16_t sampleADCA(void)
{
	int16_t sample;

    //Force start of conversion on SOC0
    AdcaRegs.ADCSOCFRC1.all = 0x03;

    //Wait for end of conversion.
    while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0){}  //Wait for ADCINT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;        //Clear ADCINT1

    //Get ADC sample result from SOC0
    sample = AdcaResultRegs.ADCRESULT1;

    return(sample);

}

// sample ADC-B
int16_t sampleADCB(void)
{
	int16_t sample;

    //Force start of conversion on SOC0
    AdcbRegs.ADCSOCFRC1.all = 0x03;

    //Wait for end of conversion.
    while(AdcbRegs.ADCINTFLG.bit.ADCINT1 == 0){}  //Wait for ADCINT1
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;        //Clear ADCINT1

    //Get ADC sample result from SOC0
    sample = AdcbResultRegs.ADCRESULT1;

    return(sample);

}

// setup blinkys
void setup_blinkys(void)
{
    EALLOW;
    GPIO_SetupPinOptions(BLINKY_LED_1, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(BLINKY_LED_1, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLINKY_LED_2, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(BLINKY_LED_2, GPIO_MUX_CPU1, 0);
    EDIS;
	return;
}

// setup timer
void setup_timer(void)
{
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
	return;
}

// setup adcs
void setup_ADCs(void)
{
	EALLOW;

	//write configurations ADCA
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
	AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

	//Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;

	//power up the ADCs
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;

	//delay for 1ms to allow ADC time to power up
	DELAY_US(1000);

	//ADCA
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 14;  //SOC0 will convert pin ADCIN14
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcaRegs.ADCSOC1CTL.bit.CHSEL = 14;  //SOC1 will convert pin ADCIN14
	AdcaRegs.ADCSOC1CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared

	AdcbRegs.ADCSOC0CTL.bit.CHSEL = 15;  //SOC0 will convert pin ADCIN14
	AdcbRegs.ADCSOC0CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcbRegs.ADCSOC1CTL.bit.CHSEL = 15;  //SOC1 will convert pin ADCIN14
	AdcbRegs.ADCSOC1CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
	AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared

	return;
}

// twiddle leds
void twiddle_blinkys(void)
{
    GPIO_WritePin(BLINKY_LED_1, 0);
    GPIO_WritePin(BLINKY_LED_2, 1);
    // TODO: setup toggling based on input gpio
    for(;;){
        GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;
        GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
        DELAY_US(50000);
    }
}
