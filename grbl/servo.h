/*
  servo.c - addition of arduino Servo-like code so GRBL can control some servos
  uses interrups and unused Timer5
  not part of Grbl, added by Alnwlsn in 2021
*/

#include "grbl.h"

#define numberOfServos 8
struct servo_t{
  uint8_t cmdParse; //hacked into the parser - activates on the line when the special servo command code is being parsed
  float parseTime;
  uint16_t moveSteps; //total number of steps to move
  uint16_t step; //current step
  uint8_t moving; //[bool] used within the ISR to move the servos
  float targetPulse[numberOfServos];
  float initPulse[numberOfServos];
  float pulseDt[numberOfServos];
  uint8_t parseInuse[numberOfServos];
  float parseValue[numberOfServos];
  uint16_t pulseTime[numberOfServos];
};
struct servo_t servo;
  
void servo_init(); //initializes the servo code

void servoPulseLimiter(); //call whenever you update the servo target, which forces it into range

void servoMoveDirect(); //"instantly" set servos to a position

void servoMoveLinear(); //move the servos smoothly over a given time