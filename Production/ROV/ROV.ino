//This code was written by, and is owned by, Jasper K. Rubin.
//Copyright 2018, All Rights Reserved.
//If you are using this code and are not Jasper Rubin, and don't have my permission, I'm mildly annoyed with you.
//If you are using this code and are not Jasper Rubin, and do have my permission, I'm still mildly annoyed with you.

#include <SoftwareSerial.h>

#define CHANNELLR 1
#define CHANNELFB 2
#define CHANNELUD 3
#define CHANNELROTATION 4
#define CHANNELPITCH 5
#define CHANNELCLAWONE 6
#define CHANNELAUTOLEVEL 7
#define CHANNELCLAWTWO 8

#define MAXRXPin 13
#define MAXTXPin 12
#define MAXControl 3

#define MAXTX HIGH
#define MAXRX LOW

#define CHANNELS 8

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

SoftwareSerial MAX(MAXRXPin, MAXTXPin);

void setup(){
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
  if(lastReceived[CHANNELPITCH] == 128){
    writeMotor(MOTORUF, lastReceived[CHANNELUD]);
    writeMotor(MOTORUB, lastReceived[CHANNELUD]);
  }else{
    writeMotor(MOTORUF, abs(lastReceived[CHANNELPITCH] - 254));
    writeMotor(MOTORUB, lastReceived[CHANNELPITCH]);
  }
}

void setClawMotors(){
  writeMotor(VERTCLAW, (((lastReceived[CHANNELCLAWONE] - 128) / 2) + 128));
  writeMotor(HORZCLAW, (((lastReceived[CHANNELCLAWTWO] - 128) / 2) + 128));
}

void setLevelMotors(){
  double x = (lastReceived[CHANNELLR] - 128);
  double y = (lastReceived[CHANNELFB] - 128);
  double theta = atan2(y, x);
  while(theta < 0){
    theta += (2 * PI);
  }
  theta *= (180 / PI);
  double throttle = sqrt((x * x) + (y * y));
  Serial.println(String(theta));
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
  if(channel <= 0 || input <= 0 || input == 255 || channel > CHANNELS){
    return;
  }
  lastReceived[channel] = input;
  
  if(channel == CHANNELCLAWONE){
    writeMotor(HORZCLAW, 128);
    lastReceived[CHANNELCLAWTWO] = 128;
  }else if(channel == CHANNELCLAWTWO){
    writeMotor(VERTCLAW, 128);
    lastReceived[CHANNELCLAWONE] = 128;
  }
}

