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
#define RESULTS_BUFFER_SIZE 8

//setup cpu1
void setup_cpu1(void);
void setup_cpu2(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
