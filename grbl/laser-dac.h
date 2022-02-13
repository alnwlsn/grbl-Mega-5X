//alnwlsn's laser control - for my K40, uses an 
//I2C DAC chip (MCP4725) to output the analog signal to the CO2 laser controller

#ifndef laserDac_h
#define laserDac_h

#include "grbl.h"

void laserDAC_init(); //initializes the servo code
void laserDACOut(uint16_t power);  //sends 0-4095 to DAC

#endif