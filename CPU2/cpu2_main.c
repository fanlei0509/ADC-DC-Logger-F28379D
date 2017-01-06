// system include
#include "F28x_Project.h"
#include "ff.h"
#include "diskio.h"

// local include
#include "setup.h"

volatile Uint16 c2_test_running;
volatile Uint16 c2_buf_1_write_count;
volatile Uint16 c2_buf_1_output_count;
BYTE c2_results_buf_1[BUFF_SIZE];
BYTE c2_results_buf_2[BUFF_SIZE];
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

#ifdef _FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

	// Initialize System Control:
	InitSysCtrl();

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

	// Wait until Shared RAM is available.
	while(!(MemCfgRegs.GSxMSEL.bit.MSEL_GS15 &
	        MemCfgRegs.GSxMSEL.bit.MSEL_GS14)){}

	// Enable global Interrupts and higher priority real-time debug events:
	EINT;  // Enable Global interrupt INTM
	ERTM;  // Enable Global realtime interrupt DBGM
#if 0
	FRESULT fresult;
    fresult = f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    //if(f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    	//twiddle_blinkys();
    fresult = f_open(&Fil, "data.txt", FA_WRITE | FA_CREATE_ALWAYS);
    //fresult = f_expand(&Fil, MAX_FILE_SIZE, 1);
	Uint16 time_data[2];
	time_data[0] = 12345;
	time_data[1] = 23456;
	Uint16 single_var = 1354;
	char test_write[4] = "test";
	Uint16 dummy;
	BYTE data[2];
	data[0] = (BYTE)(single_var >> 8);
	data[1] = (BYTE)single_var;
	f_write(&Fil, data, 2, &dummy);
	f_close(&Fil);

	fresult = f_open(&Fil, "data.txt", FA_READ);
	Uint16 time_data2[2];
	Uint16 single_var2 = 0;
	BYTE data_read[2];
	char test_write2[4];

	fresult = f_read(&Fil, data_read, 2, &dummy);
	single_var2 = data_read[1] | (Uint16)data_read[0] << 8;
	f_close(&Fil);

	buf[1] = (BYTE)(arg >> 24);		/* Argument[31..24] */
	buf[2] = (BYTE)(arg >> 16);		/* Argument[23..16] */
	buf[3] = (BYTE)(arg >> 8);		/* Argument[15..8] */
	buf[4] = (BYTE)arg;				/* Argument[7..0] */
#endif
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
    // setup data file
	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    //if(f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    	//twiddle_blinkys();
	f_open(&Fil, "data.bin", FA_WRITE | FA_CREATE_ALWAYS);
	f_expand(&Fil, MAX_FILE_SIZE, 1);

    //BYTE drv = Fil.obj.fs->drv;
    //DWORD sect = Fil.obj.fs->database + Fil.obj.fs->csize * (Fil.obj.sclust - 2);

    Uint16 data_pt_1 = TRUE;
	data_pointer = c2_results_buf_1;
	Uint16 dummy;

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
	    //disk_write(drv, data_pointer, sect, NUM_SECTIONS);
	    //sect += NUM_SECTIONS;
	    f_write(&Fil, data_pointer, BUFF_SIZE, &dummy);

		// flip to other buffer
		if(data_pt_1 == TRUE){
			data_pt_1 = FALSE;
			data_pointer = c2_results_buf_2;
			c2_buf_1_output_count++;
		}
		else{
			data_pt_1 = TRUE;
			data_pointer = c2_results_buf_1;
			c2_buf_2_output_count++;
		}

	}

	f_close(&Fil);

	return;
}
