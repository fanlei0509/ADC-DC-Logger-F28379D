// system include
#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"

#ifdef CPU1
// cpu1 setup
void setup_cpu1(void){

    // If running from flash copy RAM only functions to RAM
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

	// Initialize System Control:
	InitSysCtrl();

#ifdef _STANDALONE
#ifdef _FLASH

	// send boot command to allow the CPU2 application to begin execution
	IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
#else

	// send boot command to allow the CPU2 application to begin execution
	IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_RAM);
#endif
#endif

	// call Flash Initialization to setup flash waitstates
	#ifdef _FLASH
	   InitFlash();
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

	// Enable global Interrupts and higher priority real-time debug events:
	EINT;  // Enable Global interrupt INTM
	ERTM;  // Enable Global realtime interrupt DBGM

}
#endif

// cpu2 setup
void setup_cpu2(void){

    // If running from flash copy RAM only functions to RAM
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

	// Initialize System Control:
	InitSysCtrl();

	// call Flash Initialization to setup flash waitstates
#ifdef _FLASH
    InitFlash();
#endif

	// Clear all interrupts and initialize PIE vector table
	DINT;

	// Initialize the PIE control registers to their default state.
	InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
	InitPieVectTable();

	// Enable global Interrupts and higher priority real-time debug events:
	EINT;  // Enable Global interrupt INTM
	ERTM;  // Enable Global realtime interrupt DBGM

}
