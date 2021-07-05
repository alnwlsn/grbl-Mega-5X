//alnwlsn's laser control - uses atmega Pin 45 with 9 bit (0-512) 4Khz PWM to control a laser
//timer 5 is used

#include "grbl.h"

void laser_init(); //initializes the servo code
void laser_duty(uint16_t duty); 