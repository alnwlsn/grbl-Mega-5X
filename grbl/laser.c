#include "grbl.h"

void laser_duty(uint16_t duty){
    if(duty==0){
        TCCR5A &= ~(1<<COM5B1); //turn off PWM
        PORTL &= ~(1<<PL4); //pin 45 low (laser off)
    }else{
        if(duty>=512){duty=512;}
        TCCR5A |= (1<<COM5B1);
        OCR5B = duty - 1;
        //for 8 bit
        //if(duty>=256){duty=256;}
        //TCCR5A |= (1<<COM5B1); //turn on PWM
        //duty = duty >> 1; //shift to 8 bits
        //OCR5B = duty - 1;
    }
}

void laser_init(){
    DDRL |= (1<<PL4);
    //PORTL |= (1<<PL4);
    //TCCR5A - laser control register a
    //9 bit Fast PWM, 512 levels at ~ 4khz
    //output compare to toggle oc5b pin
    //control (0<<COM5B1) to turn on and off the pwm mode
    TCCR5A=(0<<COM5A1) | (0<<COM5A0) | (0<<COM5B1) | (0<<COM5B0) | (0<<COM5C1) | (0<<COM5C0) | (1<<WGM51) | (0<<WGM50);
    //TCCR5B - laser control register b
    //input noise canceler off icnc=0
    //input capture edge = don't care, arent using, 0
    //clock select = clk/8
    TCCR5B=(0<<ICNC5) | (0<<ICES5) | (0<<WGM53) | (1<<WGM52) | (0<<CS52) | (1<<CS51) | (0<<CS50);
    //set ocr5b between 0 and 1023 to set the pwm duty cycle

    OCR5B=0; //duty cycle (between 0 and 511)

    laser_duty(0);
}

