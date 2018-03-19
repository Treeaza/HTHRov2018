/*
Copyright 2018, Jasper Rubin
Written for Holy Trinity High's Underwater Robotics Team
All rights to the code lie with them.s

This program is written to run on an Arduino Mega or similar with a MAX485 connected
through the pins listed below, as well as motor controllers, again as listed.

PINOUT:

  - 13 - Receive from MAX
  - 12 - Transmit to MAX
  - 3 - Control MAX
  - 4 - LF motor
  - 5 - RF motor
  - 6 - RB motor
  - 7 - LB motor
  - 10 - Horizontal claw motor
  - 11 - Vertical claw motor

COMMUNICATION STRUCTURE:

  We can only read one byte at a time, so we send down a byte represtenting
  the axis we wish to move in, then the value as a second unsigned byte. Sorting out how to use
  that information is the ROV Arduino's job. In between each command we send a 0.

  We NEVER send a 0 as data, as that is used to tell the ROV we are at the beginning of a command,
  and therefore cannot be used for data.

  AXES:

    1: Left and right.
    2: Forwards and backwards.
    3: Up and down.
    4: Rotate left and right.
    5: Pitch forwards and backwards.
    6: Open and close claw.
    7: Enable/disable auto level (should only ever be sent a 1 or a 2).
    8: Open and close claw 2.
    9. Change LED mode.

*/

#include <SoftwareSerial.h>

//Channel definitions:
#define CHANNELLR 1
#define CHANNELFB 2
#define CHANNELUD 3
#define CHANNELROTATION 4
#define CHANNELPITCH 5
#define CHANNELCLAWONE 6
#define CHANNELAUTOLEVEL 7
#define CHANNELCLAWTWO 8

//Pins for connecting MAX485 to.
#define MAXRXPin 13
#define MAXTXPin 12
#define MAXControl 3

//Some constants to make serial direction control easier.
#define MAXTX HIGH
#define MAXRX LOW

//How many channels we have for control. Any channel over this must be an error.
#define CHANNELS 8

//Naming convention for motor pins is MOTOR<L/R/U><F/B>. (Left/right/up-down, front/back).
#define MOTORLF 4
#define MOTORRF 5
#define MOTORRB 6
#define MOTORLB 7
#define MOTORUF 8
#define MOTORUB 9

#define HORZCLAW 10
#define VERTCLAW 11

int motorPins[] = {MOTORLF, MOTORRF, MOTORRB, MOTORLB, MOTORUF, MOTORUB, HORZCLAW, VERTCLAW};
int motorCount = 8;
int thrusterPins[] = {MOTORLF, MOTORRF, MOTORRB, MOTORLB, MOTORUF, MOTORUB};

byte lastReceived[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

//Actual serial object.
SoftwareSerial MAX(MAXRXPin, MAXTXPin);

void setup(){
  //Set 485 to receive. We never actually need to transmit, but it's nice to have the option.
  pinMode(MAXControl, OUTPUT);
  digitalWrite(MAXControl, MAXRX);
  MAX.begin(4800);
  Serial.begin(9600);

  for(int i = 0; i < 8; i++){
    pinMode(motorPins[i], OUTPUT);
  }
}

void loop(){
  while(MAX.available()){
    byte r = MAX.read();
    if(r == 0){
      byte channel = MAX.read();
      byte input = MAX.read();
      decodeInput(channel, input);
    }
  }
  /*
  for(int i = 1; i < CHANNELS + 1; i++){
    Serial.print(lastReceived[i]);
    Serial.print(' ');
  }
  Serial.print('\n');
  */
  setLevelMotors();
  setUDMotors();
  setClawMotors();
  delay(5);
}

void setUDMotors(){
  analogWrite(MOTORUF, lastReceived[CHANNELUD]);
  //Serial.println("Front UD: " + String(lastReceived[CHANNELUD]));
  analogWrite(MOTORUB, lastReceived[CHANNELUD]);
  //Serial.println("Rear UD: " + String(lastReceived[CHANNELUD]));
}

void setClawMotors(){
  analogWrite(VERTCLAW, lastReceived[CHANNELCLAWONE]);
  //Serial.println("Vertical Claw: " + String(lastReceived[CHANNELCLAWONE]));
  analogWrite(HORZCLAW, lastReceived[CHANNELCLAWTWO]);
  //Serial.println("Horizontal Claw: " + String(lastReceived[CHANNELCLAWTWO]));
}

void setLevelMotors(){
  //The Arduino's trig functions are all in radians because reasons.
  float x = (lastReceived[CHANNELLR] - 128) / (float)128;
  float y = (lastReceived[CHANNELFB] - 128) / (float)128;
  float theta = atan2(y, x);
  if(theta < 0){
    theta += (2 * PI);
  }
  x *= (sqrt(2) / 2);
  y *= (sqrt(2) / 2);
  float throttle = 255 * sqrt((x * x) + (y * y));
  
  int octrant;
  if(theta < 23 && theta >= 337){
    quadrant = 1;
  }else if(theta >= 23 && theta < 68){
    quadrant = 2;
  }
  /*
  //RF motor first:
  float motorTheta = PI / 4;
  float motorPower = calculateMotorPower(theta, motorTheta, throttle);
  analogWrite(MOTORRF, motorPower);
  Serial.println("Motor RF: " + String(motorPower));

  //LF motor next:
  motorTheta = (3 * PI) / 4;
  motorPower = calculateMotorPower(theta, motorTheta, throttle);
  analogWrite(MOTORLF, motorPower);
  Serial.println("Motor LF: " + String(motorPower));

  //LB motor next:
  motorTheta = (5 * PI) / 4;
  motorPower = calculateMotorPower(theta, motorTheta, throttle);
  analogWrite(MOTORLB, motorPower);
  Serial.println("Motor LB: " + String(motorPower));

  //RB motor next:
  motorTheta = (7 * PI) / 4;
  motorPower = calculateMotorPower(theta, motorTheta, throttle);
  analogWrite(MOTORRB, motorPower);
  Serial.println("Motor RB: " + String(motorPower));
  */
}

float calculateMotorPower(float theta, float motorTheta, float throttle){
  return throttle * (float)(sin(abs(theta - motorTheta)));
}

void decodeInput(byte channel, byte input){
  //Something went wrong, this data is invalid.
  if(channel <= 0 || input <= 0 || input == 255 || channel > CHANNELS){
    return;
  }
  lastReceived[channel] = input;
  
  //Special case to handle claws, only one should run at a time.
  if(channel == CHANNELCLAWONE){
    analogWrite(HORZCLAW, 128);
    lastReceived[CHANNELCLAWTWO] = 128;
  }else if(channel == CHANNELCLAWTWO){
    analogWrite(VERTCLAW, 128);
    lastReceived[CHANNELCLAWONE] = 128;
  }
}

