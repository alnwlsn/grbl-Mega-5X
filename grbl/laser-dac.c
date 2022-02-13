// GRBL - alnwlsn - DAC for Laser Analog controller
// writing to an MCP4725 ADC chip
// i2c stuff from this example https://embedds.com/programming-avr-i2c-interface/

#include "grbl.h"

void laserDACOut(uint16_t dac) {  // sends 0-4095 to DAC
    if (dac >= 4095) dac = 4095;
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);  // Start
    while ((TWCR & (1 << TWINT)) == 0)
        ;
    TWDR = 0b11000000;  // addressing
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ((TWCR & (1 << TWINT)) == 0)
        ;
    TWDR = (dac >> 8) & 0x0f;  // 2nd byte
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ((TWCR & (1 << TWINT)) == 0)
        ;
    TWDR = (dac)&0xff;  // 3nd byte
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ((TWCR & (1 << TWINT)) == 0)
        ;
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);  // Stop
}

void laserDAC_init() {  // initializes the servo code
    // actually just sets up I2C
    // set SCL to 400kHz
    TWSR = 0x00;
    TWBR = 0x0C;
    // enable TWI
    TWCR = (1 << TWEN);
    laserDACOut(0);
}