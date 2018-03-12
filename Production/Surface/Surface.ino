/*
Copyright 2018, Jasper Rubin
Written for Holy Trinity High's Underwater Robotics Team
All rights to the code lie with them.

Uses the USB Host Shield 2.0 Library (http://felis.github.io/USB_Host_Shield_2.0/) by Oleg Mazurov

This program is written to run on an Arduino Uno or similar with a USB Host shield connected,
and an Xbox 360 controler attatched to that. In addition, a MAX485 module should be connected
through the pins listed below.

PINOUT:

(Digital)
 - 5 - Recieve from MAX
 - 6 - Transmit to MAX
 - 3 - Control MAX
 - 7 - Connect to RST or the controller won't work.

COMMUNICATION STRUCTURE:

  We can only read one byte at a time, so we send down a byte represtenting
  the axis we wish to move in, then the value as a second unsigned byte. Sorting out how to use
  that information is the ROV Arduino's job.

  AXES:

    1: Left and right.
    2: Forwards and backwards.
    3: Up and down.
    4: Rotate left and right.
    5: Pitch forwards and backwards.
    6: Open and close claw.
    7: Enable/disable auto level (should only ever be sent a 0 or a 1).

*/

#include <SoftwareSerial.h>
#include <XBOXUSB.h>
#include <SPI.h>

//Pins for connecting MAX485 to.
#define MAXRXPin 5
#define MAXTXPin 6
#define MAXControl 3

//Some constants to make serial direction control easier.
#define MAXTX HIGH
#define MAXRX LOW

//USB object to connect the shield, then use it to connect XBOX controller.
USB Usb;
XBOXUSB Xbox(&Usb);

//Actual serial object.
SoftwareSerial MAX(MAXRXPin, MAXTXPin);

bool autoLevelOn = true;

void setup(){
  Serial.begin(9600);
  //Set 485 to transmit. We never actually need to recieve, but it's nice to have the option.
  pinMode(MAXControl, OUTPUT);
  digitalWrite(MAXControl, MAXTX);
  //Start the connection. Baud rate doesn't really matter that much here.
  MAX.begin(4800);
  if (Usb.Init() == -1) {
    while (1);
  }
}

void loop(){
  //Does...something with the shield? I've yet to work out exactly what, but it's important.
  Usb.Task();
  if(Xbox.Xbox360Connected) {
    //alignChannel();

    //Check each of our inputs, and it there's anything cool there send it down.
    //This could be very nicely generalized but I'm not a very nice general.

    //Jack wants rotation on this one.
    readAndSendAnalogHat(LeftHatX, 4);
    //I'm told this should run up/down.
    readAndSendAnalogHat(LeftHatY, 3);
    //Right stick is all planar controls.
    readAndSendAnalogHat(RightHatX, 1);
    readAndSendAnalogHat(RightHatY, 2);
    //The triggers work subtracted from each other to actuate the claw.
    readAndSendSubtractiveTriggers(6);
    
    if(Xbox.getButtonClick(A)){
      autoLevelOn = !autoLevelOn;
    }

    bool up = Xbox.getButtonPress(UP);
    bool down = Xbox.getButtonPress(DOWN);
    
    char tiltOneZero = up - down;
    byte tilt = map(tiltOneZero, -1, 1, 0, 255);

    sendCommand(5, tilt);

    if(up || down){
      sendCommand(7, 0);
      setAutoLevelDisplay(false);
    }else{
      sendCommand(7, autoLevelOn);
      setAutoLevelDisplay(autoLevelOn);
    }
    
  }
  delay(10);
}

bool previousDisplay = false;

void setAutoLevelDisplay(bool on){
  if(on != previousDisplay){
    Xbox.setLedOff();
    if(on){
      Xbox.setLedOn(LED1);
    }else{
      Xbox.setLedOn(LED3);
    }
    previousDisplay = on;
  }
}

void readAndSendAnalogHat(AnalogHatEnum a, byte channel){
  //Read an analog hat, map it, and transmit it.
  //If it falls in the buffer zone, send 128 (the middle value).
  int value = Xbox.getAnalogHat(a);
    if(value > 8000 || value < -8000){
      byte mapped = map(value, -32768, 32768, 0, 255);
      sendCommand(channel, mapped);
    }else{
      sendCommand(channel, 128);
    }
}

void readAndSendSubtractiveTriggers(byte channel){
  //Read the trigger values and use them to find the common value, then send it.
  byte l = Xbox.getButtonPress(L2);
  byte r = Xbox.getButtonPress(R2);
  l = map(l, 0, 255, 0, 127);
  r = map(r, 0, 255, 0, 127);
  byte ret = (128 + r - l);
  sendCommand(channel, ret);
}

void alignChannel () {
  //Send a long string of 0s for reciever to check for, ensuring our communications are
  //properly aligned. Any more than 2 should work.
  for(int i = 0; i < 3; i++){
    MAX.write((byte)0);
  }
}

void sendCommand (byte axis, byte value) {
  //Just exists for convenience really.
  MAX.write(axis);
  delay(1);
  MAX.write(value);
  delay(1);
}

