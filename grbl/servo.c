/*
  servo.c - addition of arduino Servo-like code so GRBL can control some hobby servos
  uses interrups and unused Timer5
  not part of the original Grbl, added by Alnwlsn in 2021
*/

/*
8 servos connections are provided (4 on the Servos ramps header, and 4 more on the Aux-4 header):
0 - D11 PB5
1 - D6  PH3
2 - D5  PE3
3 - D4  PG5
4 - D23 PA1
5 - D25 PA3
6 - D27 PA5
7 - D29 PA7
*/

#include "grbl.h"

#define servoMinPulse 125 //~500us is min pulse, //~2500us (625) should be the max normal pulse
#define servoMaxTimeslot 627 //max possible pulse for 8 servos to coexist in a 20ms time period is 625. Also, give a couple extra ticks for good measure (ok, so it will make the period slightly longer than 20ms)
                            //therfore, the range of values in servoPulseTime should range from 0-499
static uint8_t servoISRCycle;


// //*****************************Serial debugging*******************************
// uint8_t buf[64];
// uint8_t asciiNibble(uint8_t b){ //converts nibble (0x0-0xf) from a number to an ascii character
//   if(b>0x09){
//     b+=0x37;
//   }else{
//     b+=0x30;
//   }
//   return b;
// }
// void debugSerialHex(uint8_t *buffer, uint8_t length){ //writes a hex string (in ascii) (ex: {0x01, 0x02, 0x03} -->  01 02 03 \r\n)
//   for(uint8_t y=0; y<length; y++){
//     serial_write(asciiNibble((buffer[y]>>4)&0x0f));
//     serial_write(asciiNibble((buffer[y])&0x0f));
//     serial_write(' ');
//   }
//   serial_write('\r');
//   serial_write('\n');
// }
// //************************************************************

void servoMoveDirect(){ //"instantly" set servos to a position
    for(uint8_t i=0; i<numberOfServos; i++){
      if(servo.parseInuse[i]){
        servo.pulseTime[i]=servo.parseValue[i];
      }
    }
    servoPulseLimiter();
    servo.moving=0;
}

void servoMoveLinear(){ //move the servos smoothly over a given time
    servo.moveSteps=servo.parseTime/0.020;
    for(uint8_t i=0; i<numberOfServos; i++){
      servo.initPulse[i]=servo.pulseTime[i];
      if(servo.parseInuse[i]){
        servo.targetPulse[i]=servo.parseValue[i];
        servo.pulseDt[i]=(float)(servo.targetPulse[i]-servo.pulseTime[i])/servo.moveSteps;
      }else{
        servo.pulseDt[i]=0;
      }
    }    
    servo.step=0;
}

void servoPulseLimiter(){
    for(uint8_t i=0; i<numberOfServos; i++){
        if(servo.pulseTime[i]>=499){servo.pulseTime[i]=499;}
        if(servo.pulseTime[i]<=0){servo.pulseTime[i]=0;}
    }
}

void servoPulseOn(){
    uint16_t servotemp = servoMinPulse + servo.pulseTime[servoISRCycle>>1];
    OCR5AH=(servotemp>>8)&255;
    OCR5AL=(servotemp)&255;
}
void servoPulseOff(){
    uint16_t servotemp = servoMaxTimeslot - servoMinPulse - servo.pulseTime[servoISRCycle>>1];
    OCR5AH=(servotemp>>8)&255;
    OCR5AL=(servotemp)&255;
}

ISR (TIMER5_COMPA_vect){
    PORTH |= (1<<PH1);
    switch(servoISRCycle){ //increments 2x for each servo, once for pulse on, once for pulse off
        case 0: //servo 0 pulse start
            PORTB |= (1<<PB5);
            servoPulseOn();
            break;
        case 1: //servo 0 pulse end
            PORTB &= ~(1<<PB5);
            servoPulseOff();
            break;
        case 2: //servo 1 pulse start
            PORTH |= (1<<PH3);
            servoPulseOn();
            break;
        case 3: //servo 1 pulse end
            PORTH &= ~(1<<PH3);
            servoPulseOff();
            break;
        case 4: //servo 2 pulse start
            PORTE |= (1<<PE3);
            servoPulseOn();
            break;
        case 5: //servo 2 pulse end
            PORTE &= ~(1<<PE3);
            servoPulseOff();
            break;
        case 6: //servo 3 pulse start
            PORTG |= (1<<PG5);
            servoPulseOn();
            break;
        case 7: //servo 3 pulse end
            PORTG &= ~(1<<PG5);
            servoPulseOff();
            break;
        case 8: //servo 4 pulse start
            PORTA |= (1<<PA1);
            servoPulseOn();
            break;
        case 9: //servo 4 pulse end
            PORTA &= ~(1<<PA1);
            servoPulseOff();
            break;
        case 10: //servo 5 pulse start
            PORTA |= (1<<PA3);
            servoPulseOn();
            break;
        case 11: //servo 5 pulse end
            PORTA &= ~(1<<PA3);
            servoPulseOff();
            break;
        case 12: //servo 6 pulse start
            PORTA |= (1<<PA5);
            servoPulseOn();
            break;
        case 13: //servo 6 pulse end
            PORTA &= ~(1<<PA5);
            servoPulseOff();
            break;
        case 14: //servo 7 pulse start
            PORTA |= (1<<PA7);
            servoPulseOn();
            break;
        case 15: //servo 7 pulse end
            PORTA &= ~(1<<PA7);
            servoPulseOff();
            servoISRCycle=255;
            if(servo.moving==1){
                if(servo.step>=servo.moveSteps){
                    for(uint8_t i=0;i<numberOfServos;i++){
                        servo.pulseTime[i]=servo.targetPulse[i];
                    }
                    servo.moving=0;
                }else{
                    for(uint8_t i=0;i<numberOfServos;i++){
                        servo.initPulse[i]+=servo.pulseDt[i];
                        servo.pulseTime[i]=servo.initPulse[i];
                    }
                    servo.step+=1;
                }
            }
            break;
    }
    servoISRCycle++;
    PORTH &= ~(1<<PH1);
}

void servo_init(){
    servoISRCycle = 0;
    //test for some interrupts on a timer for some servos
    DDRB |= (1<<PB5); //pins as outputs
    DDRH |= (1<<PH3);
    DDRE |= (1<<PE3);
    DDRG |= (1<<PG5);
    DDRA |= (1<<PA1);
    DDRA |= (1<<PA3);
    DDRA |= (1<<PA5);
    DDRA |= (1<<PA7);

    DDRH |= (1<<PH1); //this one is just for testing
    //TCCR5A - timer5 control register a
    //output compare pins a, b, c off (COM5xy = 0,0)
    //wgm CTC mode, clear timer on match with ocr5a = 0100
    TCCR5A=(0<<COM5A1) | (0<<COM5A0) | (0<<COM5B1) | (0<<COM5B0) | (0<<COM5C1) | (0<<COM5C0) | (0<<WGM51) | (0<<WGM50);
    //TCCR5B - timer5 control register b
    //input noise canceler off icnc=0
    //input capture edge = don't care, arent using, 0
    //clock select = clk/64 = 16mhz/64 --> 20ms = 5000 ticks; 1ms = 250 ticks | 0b011
    TCCR5B=(0<<ICNC5) | (0<<ICES5) | (0<<WGM53) | (1<<WGM52) | (0<<CS52) | (1<<CS51) | (1<<CS50);
    //current timer count values
    TCNT5H=0x00; 
    TCNT5L=0x00;
    //input capture registers not used
    ICR5H=0x00;
    ICR5L=0x00;
    //output compare register = 5000 = 0x1388 (are only using ocra) - though this really only stays this way for a little bit after init
    OCR5AH=0x13;
    OCR5AL=0x88;
    //enable interrupt on orc5a
    TIMSK5=(0<<ICIE5) | (0<<OCIE5C) | (0<<OCIE5B) | (1<<OCIE5A) | (0<<TOIE5);

    //set inital values of struct
    servo.cmdParse=0;
    for(uint8_t i=0; i<numberOfServos; i++){
        servo.parseInuse[i]=0;
        servo.parseValue[i]=0;
        servo.pulseTime[i]=0;
    }
    servoPulseLimiter();
}