// system include
#include "F28x_Project.h"
#include "ff.h"
#include "diskio.h"

// local include
#include "setup.h"

volatile Uint16 c2_test_running;
volatile Uint16 c2_buf_1_write_count;
volatile Uint16 c2_buf_1_output_count;
Uint16 c2_results_buf_1[NUM_VALUES];
Uint16 c2_results_buf_2[NUM_VALUES];
volatile Uint16 c2_buf_2_write_count;
volatile Uint16 c2_buf_2_output_count;

#pragma DATA_SECTION(c2_test_running,"SHARERAMGS1");
#pragma DATA_SECTION(c2_buf_1_write_count,"SHARERAMGS2");
#pragma DATA_SECTION(c2_buf_2_write_count,"SHARERAMGS3");

#pragma DATA_SECTION(c2_results_buf_1,"SHARERAMGS4");
#pragma DATA_SECTION(c2_results_buf_2,"SHARERAMGS5");

#pragma DATA_SECTION(c2_buf_1_output_count,"SHARERAMGS14");
#pragma DATA_SECTION(c2_buf_2_output_count,"SHARERAMGS15");

// functions
void write_data(void);

// variables
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */
BYTE* data_pointer; // = (BYTE *)results_buf_1;

// main execution code
void main(void) {

	// Initialize System Control:
	InitSysCtrl();

	// Clear all interrupts and initialize PIE vector table
	DINT;

	// Initialize the PIE control registers to their default state.
	InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
	InitPieVectTable();

	// Wait until Shared RAM is available.
	while(!(MemCfgRegs.GSxMSEL.bit.MSEL_GS15 &
	        MemCfgRegs.GSxMSEL.bit.MSEL_GS14)){}

	// Enable global Interrupts and higher priority real-time debug events:
	EINT;  // Enable Global interrupt INTM
	ERTM;  // Enable Global realtime interrupt DBGM

	// loop forever until test is running
	while(1){

		// check if test should be run
		if(c2_test_running){
		    c2_buf_1_output_count = 0;
		    c2_buf_2_output_count = 0;
			write_data();
		}
	}

}

void write_data(void)
{
	FRESULT fresult;
    // setup data file
    fresult = f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    //if(f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    	//twiddle_blinkys();
    fresult = f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS);
    fresult = f_expand(&Fil, MAX_FILE_SIZE, 1);

    BYTE drv = Fil.obj.fs->drv;
    DWORD sect = Fil.obj.fs->database + Fil.obj.fs->csize * (Fil.obj.sclust - 2);

    Uint16 data_pt_1 = TRUE;
	data_pointer = (BYTE *)c2_results_buf_1;

	// wait until data is ready to be saved to file
	while(c2_buf_1_write_count == 0){}

	// write all data
	while(1){

		// check if test is still running and all data has been written
		if((c2_test_running == FALSE) &&
		   (c2_buf_1_write_count == c2_buf_1_output_count) &&
		   (c2_buf_2_write_count == c2_buf_2_output_count))
			break;

		// make sure that there is data to write
		if(data_pt_1 == TRUE)
			while(c2_buf_1_write_count == c2_buf_1_output_count){}
		else
			while(c2_buf_2_write_count == c2_buf_2_output_count){}

		// write the data
	    disk_write(drv, data_pointer, sect, NUM_SECTIONS);
	    sect += NUM_SECTIONS;

		// flip to other buffer
		if(data_pt_1 == TRUE){
			data_pt_1 = FALSE;
			data_pointer = (BYTE *)c2_results_buf_2;
			c2_buf_1_output_count++;
		}
		else{
			data_pt_1 = TRUE;
			data_pointer = (BYTE *)c2_results_buf_1;
			c2_buf_2_output_count++;
		}

	}

	f_close(&Fil);

	return;
}
