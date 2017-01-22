#ifndef __SETUP_H__
#define __SETUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CPU2
#ifdef _FLASH

// These are defined by the linker (see device linker command file)
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadSize;
extern Uint16 RamfuncsRunStart;
#endif
#endif

// defines
#define BLINKY_LED_1        31
#define BLINKY_LED_2        34
#define TIMER_GPIO          65

#define NUM_WRITES          7500 //10000 //10
#define NUM_SAMPLES         5120
#define BUFF_SIZE           20480   // NUM_SAMPLES*3*2
#define NUM_SECTIONS        40      // BUFF_SIZE/512
#define MAX_FILE_SIZE       153600000 //307200 // NUM_WRITES*BUFF_SIZE


#define TRUE                1
#define FALSE               0

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
