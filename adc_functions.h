// lm73_functions.h 
// Roger Traylor 11.27.10
//special defines and functions for the lm73 temperature sensor

#define F_CPU 16000000UL
#define TRUE  1
#define FALSE 0

#include "twi_master.h"  //my TWCR_START, STOP, NACK, RNACK, SEND
//use the status codes in: usr/local/AVRMacPack/avr-3/include/util/twi.h

#define ADC_ADDRESS 0x10                    //LM73-0, address pin floating
#define ADC_WRITE (LM92_ADDRESS | TW_WRITE) //LSB is a zero to write
#define ADC_READ  (LM92_ADDRESS | TW_READ)  //LSB is a one to read

//special functions for lm73 temperature sensor
  
