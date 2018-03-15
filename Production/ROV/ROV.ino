/*
Copyright 2018, Jasper Rubin
Written for Holy Trinity High's Underwater Robotics Team
All rights to the code lie with them.s

This program is written to run on an Arduino Mega or similar with a MAX485 connected
through the pins listed below, as well as motor controllers, again as listed.

PINOUT:

[TODO]

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

*/

#include <SoftwareSerial.h>

//Pins for connecting MAX485 to.
#define MAXRXPin 10
#define MAXTXPin 11
#define MAXControl 3

//Some constants to make serial direction control easier.
#define MAXTX HIGH
#define MAXRX LOW

//How many channels we have for control. Any channel over this must be an error.
#define CHANNELS 7

//Actual serial object.
SoftwareSerial MAX(MAXRXPin, MAXTXPin);

void setup(){
  //Set 485 to receive. We never actually need to transmit, but it's nice to have the option.
  pinMode(MAXControl, OUTPUT);
  digitalWrite(MAXControl, MAXRX);
  MAX.begin(4800);
  Serial.begin(9600);
}

void loop(){
  if(MAX.available()){
    byte r;
    while(MAX.available()){
      r = MAX.read();
      if(r == 0){
        byte channel = MAX.read();
        byte input = MAX.read();
        decodeInput(channel, input);
      }
    }
  }
  delay(5);
}

void decodeInput(byte channel, byte input){
  //Something went wrong, this data is invalid.
  if(channel <= 0 || input <= 0 || channel > CHANNELS){
    return;
  }
}

