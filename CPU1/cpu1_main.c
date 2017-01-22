// system include
#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "ff.h"
#include "diskio.h"
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */
// local include
#include "setup.h"

volatile Uint16 c1_test_running;
volatile Uint16 c1_buf_1_write_count;
volatile Uint16 c1_buf_1_output_count;
BYTE c1_results_buf_1[BUFF_SIZE];
BYTE c1_results_buf_2[BUFF_SIZE];
volatile Uint16 c1_buf_2_write_count;
volatile Uint16 c1_buf_2_output_count;

#pragma DATA_SECTION(c1_test_running,"SHARERAMGS1");
#pragma DATA_SECTION(c1_buf_1_write_count,"SHARERAMGS2");
#pragma DATA_SECTION(c1_buf_2_write_count,"SHARERAMGS3");

#pragma DATA_SECTION(c1_results_buf_1,"SHARERAMGS4");
#pragma DATA_SECTION(c1_results_buf_2,"SHARERAMGS5");

#pragma DATA_SECTION(c1_buf_1_output_count,"SHARERAMGS14");
#pragma DATA_SECTION(c1_buf_2_output_count,"SHARERAMGS15");

// function declarations
__interrupt void cpu_timer0_isr(void);
void capture_data(Uint32 *time, Uint16 *interrupted);
inline void sampleADCA(BYTE *loc1, BYTE *loc2);
inline void sampleADCB(BYTE *loc1, BYTE *loc2);
void setup_blinkys(void);
void setup_timer(void);
void setup_ADCs(void);
void setup_fatfs(void);
void twiddle_blinkys(void);

BYTE* array_pointer;
Uint16 resultsIndex = 0;

// main execution code
void main(void) {

	// Initialize System Control:
	InitSysCtrl();

#ifdef _FLASH
	InitFlash();
#endif

#ifdef _STANDALONE
#ifdef _FLASH
    //  Send boot command to allow the CPU02 application to begin execution
    IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
#else
    //  Send boot command to allow the CPU02 application to begin execution
    IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_RAM);
#endif
#endif

	// Initialize GPIO:
	InitGpio();

	// Clear all interrupts and initialize PIE vector table
	DINT;

	// Initialize the PIE control registers to their default state.
	InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
	InitPieVectTable();

	// share ram
    while( !(MemCfgRegs.GSxMSEL.bit.MSEL_GS15 &
             MemCfgRegs.GSxMSEL.bit.MSEL_GS14))
    {
        EALLOW;
        MemCfgRegs.GSxMSEL.bit.MSEL_GS15 = 1;
        MemCfgRegs.GSxMSEL.bit.MSEL_GS14 = 1;
        EDIS;
    }

	// setup blinky 1 and 2
	setup_blinkys();

    // configure CPU-Timer 0 interrupt every microsecond:
	setup_timer();

    // setup ADCs
	setup_ADCs();

	// setup fatfs to write from cpu2
	setup_fatfs();

	// enable interrupts
    EINT;
    ERTM;

	// TODO need to setup serial port to start running test
    c1_test_running = TRUE;
    Uint32 run_time = 0;
    Uint16 interrupted = FALSE;

	// loop forever
	while(1){

		// check if test should be run
		if(c1_test_running == TRUE){
		    c1_buf_1_write_count = 0;
		    c1_buf_2_write_count = 0;
			capture_data(&run_time, &interrupted);
		}

	}

}

// timer 0 clock
__interrupt void cpu_timer0_isr(void)
{
	CpuTimer0.InterruptCount++;


	// acknowledge this interrupt to receive more interrupts from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

// capture data
void capture_data(Uint32 *time, Uint16 *interrupted)
{
    //data
    Uint16 i,j;
    Uint16 array_pt_1 = TRUE;
    Uint32 start_time, stop_time;
    array_pointer = c1_results_buf_1;

    // read NUM_WRITES*NUM_SAMPLES of data
    start_time = CpuTimer0.InterruptCount;
	for(j = 0; j < NUM_WRITES; ++j){

		// make sure that buffer has been output to data file before writing to
		if(array_pt_1 == TRUE)
			while(c1_buf_1_write_count != c1_buf_1_output_count){(*interrupted) = TRUE;}
		else
			while(c1_buf_2_write_count != c1_buf_2_output_count){(*interrupted) = TRUE;}

		// populate specified buffer
		for(i = 0; i < NUM_SAMPLES; ++i){
			// TODO hopefully can make this 0
			DELAY_US(1);//DELAY_US(1);

			//array_pointer[resultsIndex] = (BYTE)(CpuTimer0.InterruptCount16 >> 8);
			//array_pointer[resultsIndex+1] = (BYTE)CpuTimer0.InterruptCount16;
			//array_pointer[resultsIndex+1] = sampleADCA();
			//array_pointer[resultsIndex+2] = sampleADCB();
			sampleADCA(&array_pointer[resultsIndex], &array_pointer[resultsIndex+1]);
			sampleADCB(&array_pointer[resultsIndex+2], &array_pointer[resultsIndex+3]);
			//resultsIndex = resultsIndex + 6;
			resultsIndex = resultsIndex + 4;
		}

		// reset index and flip to other buffer
		resultsIndex = 0;
		if(array_pt_1 == TRUE){
			array_pt_1 = FALSE;
			array_pointer = c1_results_buf_2;
			c1_buf_1_write_count++;
		}
		else{
			array_pt_1 = TRUE;
			array_pointer = c1_results_buf_1;
			c1_buf_2_write_count++;
		}

		// TODO check for serial communication to end test
		//if(j==100)
			//break;
	}
    stop_time = CpuTimer0.InterruptCount;
    (*time) = stop_time - start_time;
	// reached maximum length
	c1_test_running = FALSE;

	return;
}

// sample ADC-A
void sampleADCA(BYTE *loc1, BYTE *loc2)
{
    //Force start of conversion on SOC0
    AdcaRegs.ADCSOCFRC1.all = 0x03;

    //Wait for end of conversion.
    while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0){}  //Wait for ADCINT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;        //Clear ADCINT1

    //Get ADC sample result from SOC0
    (*loc1) = (BYTE)(AdcaResultRegs.ADCRESULT0 >> 8);
    (*loc2) = (BYTE)AdcaResultRegs.ADCRESULT0;

    return;

}

// sample ADC-B
void sampleADCB(BYTE *loc1, BYTE *loc2)
{
    //Force start of conversion on SOC0
    AdcbRegs.ADCSOCFRC1.all = 0x03;

    //Wait for end of conversion.
    while(AdcbRegs.ADCINTFLG.bit.ADCINT1 == 0){}  //Wait for ADCINT1
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;        //Clear ADCINT1

    //Get ADC sample result from SOC0
    (*loc1) = (BYTE)(AdcbResultRegs.ADCRESULT0 >> 8);
    (*loc2) = (BYTE)AdcbResultRegs.ADCRESULT0;

    return;

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
    //AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    //AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);


	//Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
	AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;

	//power up the ADCs
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
	AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;

	//delay for 1ms to allow ADC time to power up
	DELAY_US(1000);

	//ADCA
#if 0
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 14;  //SOC0 will convert pin ADCIN14
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcaRegs.ADCSOC1CTL.bit.CHSEL = 14;  //SOC1 will convert pin ADCIN14
	AdcaRegs.ADCSOC1CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared

	AdcbRegs.ADCSOC0CTL.bit.CHSEL = 15;  //SOC0 will convert pin ADCIN15
	AdcbRegs.ADCSOC0CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcbRegs.ADCSOC1CTL.bit.CHSEL = 15;  //SOC1 will convert pin ADCIN15
	AdcbRegs.ADCSOC1CTL.bit.ACQPS = 25; //sample window is acqps + 1 SYSCLK cycles
	AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
	AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
	EDIS;
#endif
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert pin ADCIN14
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 63; //sample window is acqps + 1 SYSCLK cycles
	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared

	AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2;  //SOC0 will convert pin ADCIN15
	AdcbRegs.ADCSOC0CTL.bit.ACQPS = 63; //sample window is acqps + 1 SYSCLK cycles
	AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
	AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
	EDIS;

	return;
}

// setup fatfs
void setup_fatfs(void)
{
	EALLOW;
#if 0
	GpioCtrlRegs.GPDPUD.bit.GPIO124 = 0;
	GpioCtrlRegs.GPDMUX2.bit.GPIO124 = 0;
	GpioCtrlRegs.GPDDIR.bit.GPIO124 = 0;
	//GPIO_SetupPinMux(124, GPIO_MUX_CPU2, 0);

	GpioCtrlRegs.GPDPUD.bit.GPIO122 = 0;
	GpioDataRegs.GPDSET.bit.GPIO122 = 1;
	GpioCtrlRegs.GPDMUX2.bit.GPIO122 = 0;
	GpioCtrlRegs.GPDDIR.bit.GPIO122 = 1;
	//GPIO_SetupPinMux(122, GPIO_MUX_CPU2, 0);

	GpioCtrlRegs.GPDPUD.bit.GPIO123 = 0;
	GpioDataRegs.GPDSET.bit.GPIO123 = 1;
	GpioCtrlRegs.GPDMUX2.bit.GPIO123 = 0;
	GpioCtrlRegs.GPDDIR.bit.GPIO123 = 1;
	//GPIO_SetupPinMux(123, GPIO_MUX_CPU2, 0);

	GpioCtrlRegs.GPDPUD.bit.GPIO125 = 0;
	GpioDataRegs.GPDSET.bit.GPIO125 = 1;
	GpioCtrlRegs.GPDMUX2.bit.GPIO125 = 0;
	GpioCtrlRegs.GPDDIR.bit.GPIO125 = 1;
	//GPIO_SetupPinMux(125, GPIO_MUX_CPU2, 0);
#endif
    GPIO_SetupPinOptions(124, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(124, GPIO_MUX_CPU2, 0);

    GPIO_SetupPinOptions(122, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(122, GPIO_MUX_CPU2, 0);

    GPIO_SetupPinOptions(123, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(123, GPIO_MUX_CPU2, 0);

    GPIO_SetupPinOptions(125, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(125, GPIO_MUX_CPU2, 0);

	EDIS;
#if 0
	FRESULT fresult;
    // setup data file
    fresult = f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    //if(f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    	//twiddle_blinkys();
    fresult = f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS);
    fresult = f_expand(&Fil, MAX_FILE_SIZE, 1);
	f_close(&Fil);
#endif
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
