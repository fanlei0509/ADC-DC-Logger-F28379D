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

#define NUM_WRITES          10
#define NUM_SAMPLES         5120
#define NUM_VALUES          15360   // NUM_SAMPLES*3
#define BUFF_SIZE           30720   // NUM_SAMPLES*3*2
#define NUM_SECTIONS        60      // BUFF_SIZE/512
#define MAX_FILE_SIZE       307200 // NUM_WRITES*BUFF_SIZE

#define TRUE                1
#define FALSE               0

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
