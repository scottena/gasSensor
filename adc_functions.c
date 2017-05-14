// lm73_functions.c       
// Roger Traylor 11.28.10

#include <util/twi.h>
#include "lm92_functions.h"
#include <util/delay.h>

//TODO: remove volatile type modifier?  I think so.
//TODO: initalize with more resolution and disable the smb bus timeout
//TODO: write functions to change resolution, alarm etc.

volatile uint8_t adc_wr_buf;
volatile uint8_t adc_rd_buf[2];

