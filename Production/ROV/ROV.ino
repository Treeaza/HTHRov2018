<<<<<<< HEAD
//This code was written by, and is owned by, Jasper K. Rubin.
//Copyright 2018, All Rights Reserved.
//If you are using this code and are not Jasper Rubin, and don't have my permission, I'm mildly annoyed with you.
//If you are using this code and are not Jasper Rubin, and do have my permission, I'm still mildly annoyed with you.
=======
/*
Copyright 2018, Jasper Rubin
Written for Holy Trinity High's Underwater Robotics Team
All rights to the code lie with them.

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
    --DEPRECATED-- 7: Enable/disable auto level (should only ever be sent a 1 or a 2).
    8: Open and close claw 2.

*/
>>>>>>> parent of abe886b... Update ROV.ino

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

#define LEDPIN 3

int motorPins[] = {MOTORLF, MOTORRF, MOTORRB, MOTORLB, MOTORUF, MOTORUB, HORZCLAW, VERTCLAW};
int motorCount = 8;
int thrusterPins[] = {MOTORLF, MOTORRF, MOTORRB, MOTORLB, MOTORUF, MOTORUB};

byte lastReceived[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

//Actual serial object.
SoftwareSerial MAX(MAXRXPin, MAXTXPin);

void setup(){
  //Serial.begin(9600);
  //Set 485 to receive. We never actually need to transmit, but it's nice to have the option.
  pinMode(MAXControl, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(MAXControl, MAXRX);
  MAX.begin(74880);
  
  for(int i = 0; i < 8; i++){
    pinMode(motorPins[i], OUTPUT);
  }
}

void loop(){
  digitalWrite(LEDPIN, HIGH);
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
    writeMotor(MOTORUB, lastReceived[CHANNELUD]);
  }else{
    //If we are running a pitch control, ignore UD and instead just go with the pitch.
    writeMotor(MOTORUF, abs(lastReceived[CHANNELPITCH] - 254));
    writeMotor(MOTORUB, lastReceived[CHANNELPITCH]);
  }
}

void setClawMotors(){
  //Remarkably, this all functions pretty much correctly.
  //One caveat: claws are jamming shut, so we're cutting claw power by 2 to try and fix this.
  writeMotor(VERTCLAW, (((lastReceived[CHANNELCLAWONE] - 128) / 2) + 128));
  writeMotor(HORZCLAW, (((lastReceived[CHANNELCLAWTWO] - 128) / 2) + 128));
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
  Serial.println(String(theta));
  //This great and terrible if statement determines which motors to turn on based on angle.
  int lf = 0, rf = 0, lb = 0, rb = 0;
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
  //Serial.println("LF:" + String(lf) + ", RF:" + String(rf) + ", LB:" + String(lb) + ", RB:" + String(rb));
  //Now we have to deal with rotation.
  
  byte rotValue = lastReceived[CHANNELROTATION];
  if(rotValue != 128){
    lf = (lf + (255 - rotValue)) / 2;
    lb = (lb + (255 - rotValue)) / 2;
    rf = (rf + rotValue) / 2;
    rb = (rb + rotValue) / 2;
  }
  
  writeMotor(MOTORLF, lf);
  writeMotor(MOTORRF, rf);
  writeMotor(MOTORLB, lb);
  writeMotor(MOTORRB, rb);
  
}

void writeMotor(int pin, float value){
  value = (int)constrain(value, 0, 255);
  if(value == 128){
    value += 2;
  }
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

