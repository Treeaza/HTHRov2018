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
  setLevelMotors();
  setUDMotors();
  setClawMotors();
  delay(5);
}

void setUDMotors(){
  //If no pitch control, just set the UD motors to whatever we read on UD.
  if(lastReceived[CHANNELPITCH] == 128){
    writeMotor(MOTORUF, lastReceived[CHANNELUD]);
    //Serial.println("Front UD: " + String(lastReceived[CHANNELUD]));
    writeMotor(MOTORUB, lastReceived[CHANNELUD]);
    //Serial.println("Rear UD: " + String(lastReceived[CHANNELUD]));
  }else{
    //If we are running a pitch control, ignore UD and instead just go with the pitch.
    writeMotor(MOTORUF, abs(lastReceived[CHANNELPITCH] - 254));
    //Serial.println("Front UD: " + String(abs(lastReceived[CHANNELPITCH] - 254)));
    writeMotor(MOTORUB, lastReceived[CHANNELPITCH]);
    //Serial.println("Rear UD: " + String(lastReceived[CHANNELPITCH]));
  }
}

void setClawMotors(){
  //Remarkably, this all functions pretty much correctly.
  writeMotor(VERTCLAW, lastReceived[CHANNELCLAWONE]);
  //Serial.println("Vertical Claw: " + String(lastReceived[CHANNELCLAWONE]));
  writeMotor(HORZCLAW, lastReceived[CHANNELCLAWTWO]);
  //Serial.println("Horizontal Claw: " + String(lastReceived[CHANNELCLAWTWO]));
}

void setLevelMotors(){
  //The Arduino's trig functions are all in radians because reasons.
  double x = (lastReceived[CHANNELLR] - 128);
  double y = (lastReceived[CHANNELFB] - 128);
  double theta = atan2(y, x);
  while(theta < 0){
    theta += (2 * PI);
  }
  theta *= (180 / PI);
  double throttle = sqrt((x * x) + (y * y));

  //This great and terrible if statement determines which motors to turn on based on angle.
  float lf = 0, rf = 0, lb = 0, rb = 0;
  if(theta < 23 || theta >= 337){
    rf = -1;
    lb = -1;
    rb = 1;
    lf = 1;
  }else if(theta >= 23 && theta < 68){
    rf = 0;
    lb = 0;
    rb = 1;
    lf = 1;
  }else if(theta >= 68 && theta < 113){
    rf = 1;
    lf = 1;
    lb = 1;
    rb = 1;
  }else if(theta >= 113 && theta < 157){
    lf = 0;
    rb = 0;
    lb = 1;
    rf = 1;
  }else if(theta >= 157 && theta < 202){
    lf = -1;
    rb = -1;
    lb = 1;
    rf = 1;
  }else if(theta >= 202 && theta < 247){
    lf = -1;
    rb = -1;
    lb = 0;
    rf = 0;
  }else if(theta >= 247 && theta < 293){
    lf = -1;
    rb = -1;
    lb = -1;
    rf = -1;
  }else{
    lf = 0;
    rb = 0;
    lb = -1;
    rf = -1;
  }
  lf *= throttle;
  rf *= throttle;
  rb *= throttle;
  lb *= throttle;
  
  lf += 128;
  rf += 128;
  rb += 128;
  lb += 128;
  //Now we have to deal with rotation.
  byte rotValue = lastReceived[CHANNELROTATION];
  if(rotValue != 128){
    Serial.println(rotValue);
    lf = (lf + (255 - rotValue)) / 2;
    lb = (lb + (255 - rotValue)) / 2;
    rf = (rf + rotValue) / 2;
    rb = (rb + rotValue) / 2;
  }
  writeMotor(MOTORLF, lf);
  Serial.println("MOTORLF: " + String(lf));
  writeMotor(MOTORRF, rf);
  Serial.println("MOTORRF: " + String(rf));
  writeMotor(MOTORLB, lb);
  Serial.println("MOTORLB: " + String(lb));
  writeMotor(MOTORRB, rb);
  Serial.println("MOTORRB: " + String(rb));
  
}

void writeMotor(int pin, float value){
  analogWrite(pin, (int)constrain(value, 0, 255));
}

void decodeInput(byte channel, byte input){
  //Something went wrong, this data is invalid.
  if(channel <= 0 || input <= 0 || input == 255 || channel > CHANNELS){
    return;
  }
  lastReceived[channel] = input;
  
  //Special case to handle claws, only one should run at a time.
  if(channel == CHANNELCLAWONE){
    writeMotor(HORZCLAW, 128);
    lastReceived[CHANNELCLAWTWO] = 128;
  }else if(channel == CHANNELCLAWTWO){
    writeMotor(VERTCLAW, 128);
    lastReceived[CHANNELCLAWONE] = 128;
  }
}

