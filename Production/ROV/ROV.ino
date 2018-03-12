/*
Copyright 2018, Jasper Rubin
Written for Holy Trinity High's Underwater Robotics Team
All rights to the code lie with them.s

This program is written to run on an Arduino Mega or similar with a MAX485 connected
through the pins listed below, as well as motor controllers, again as listed.

PINOUT:

[TODO]

COMMUNICATION STRUCTURE:

  We can only read one byte at a time, so we send down a character represtenting
  the axis we wish to move in, then the value as a second unsigned byte. Sorting out how to use
  that information is the ROV Arduino's job.

  AXES:

    X: Left and right.
    Y: Forwards and backwards.
    Z: Up and down.
    R: Rotate left and right.
    P: Pitch forwards and backwards.
    C: Open and close claw.
    G: Enable/disable auto level (should only ever be sent a 0 or a 1).

*/

#include <SoftwareSerial.h>

//Pins for connecting MAX485 to.
#define MAXRXPin 11
#define MAXTXPin 10
#define MAXControl 3

//Some constants to make serial direction control easier.
#define MAXTX HIGH
#define MAXRX LOW

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
    }
    Serial.println(r);
  }
  //At some point handle auto-level.
  delay(5);
}

void decodeInput(byte channel, byte input){
  Serial.print("Input on channel ");
  Serial.print(channel);
  Serial.print(" of ");
  Serial.print(input);
  Serial.print('\n');
}

