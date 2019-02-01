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
 - 4 - Connect to LED to show status.

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
    --DEPRECATED-- 7: Enable/disable auto level (should only ever be sent a 1 or a 2).
    8: Open and close claw 2.

NOTE:

  Due to rate-limiting things with the 485, if you provide too much input at once the controller may freeze,
  apparently just as a result of trying to send too many data at once. To solve this, I recommend putting a pushbutton
  on the Arduino's RST line to reset it when this happens.

*/
>>>>>>> parent of de4de1a... Update Surface.ino

#include <SoftwareSerial.h>
#include <XBOXUSB.h>
#include <SPI.h>

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
#define MAXRXPin 5
#define MAXTXPin 6
#define MAXControl 3

//Some constants to make serial direction control easier.
#define MAXTX HIGH
#define MAXRX LOW

#define CHANNELS 8
#define LASTSENTRESETCOUNT 500

#define STATUSLEDPIN 4

//USB object to connect the shield, then use it to connect XBOX controller.
USB Usb;
XBOXUSB Xbox(&Usb);

//Actual serial object.
SoftwareSerial MAX(MAXRXPin, MAXTXPin);

bool clawMode = true;

void setup(){
  //Set 485 to transmit. We never actually need to receive, but it's nice to have the option.
  pinMode(MAXControl, OUTPUT);
  pinMode(STATUSLEDPIN, OUTPUT);
  digitalWrite(STATUSLEDPIN, LOW);
  digitalWrite(MAXControl, MAXTX);
  //Start the connection. "Baud rate doesn't really matter that much here." - Jasper Rubin, 2018
  //Later 2018 Jasper here to say it turns out baud rate is really f***ing important here.
  //Higher is better, lower than this and the controller starts freezing up.
  MAX.begin(74880);
  if (Usb.Init() == -1) {
    while (1);
  }
}

int lastSentResetCounter = LASTSENTRESETCOUNT;

bool claw = false;

byte lastSent[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

void loop(){
  //If we got here, it all initialized properly.
  //Does...something with the shield? I've yet to work out exactly what, but it's important.
  Usb.Task();
  if(Xbox.Xbox360Connected) {
    
    digitalWrite(STATUSLEDPIN, HIGH);
    
    //Periodically reset lastSent, to ensure fresh info every few seconds at least.
    lastSentResetCounter--;
    if(lastSentResetCounter <= 0){
      for(int i = 0; i < CHANNELS; i++){
        lastSent[i] = 0;
      }
      lastSentResetCounter = LASTSENTRESETCOUNT;
    }
    
    //Check each of our inputs, and it there's anything cool there send it down.
    //This could be very nicely generalized but I'm not a very nice general.

    //Jack wants rotation on this one.
    readAndSendAnalogHat(LeftHatX, CHANNELROTATION);
    //I'm told this should run up/down.
    readAndSendAnalogHat(LeftHatY, CHANNELUD);
    //Right stick is all planar controls.
    readAndSendAnalogHat(RightHatX, CHANNELLR);
    readAndSendAnalogHat(RightHatY, CHANNELFB);
   
    //The triggers work subtracted from each other to actuate the claw.
    readAndSendSubtractiveTriggers(claw? CHANNELCLAWONE: CHANNELCLAWTWO);

    //Change claw mode.
    if(Xbox.getButtonClick(A)){
      claw = !claw;
    }
    setClawModeDisplay(claw);

    //Tilting and autolevel.
    bool up = Xbox.getButtonPress(UP);
    bool down = Xbox.getButtonPress(DOWN);
   
    char tiltOneZero = up - down;
    byte tilt = map(tiltOneZero, -1, 1, 1, 255);

    sendCommand(5, tilt);
  }
  delay(1);
}

bool previousDisplay = false;

void setClawModeDisplay(bool on){
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
    byte mapped = map(value, -32768, 32768, 1, 254);
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

void sendCommand (byte axis, byte value) {
  //Just exists for convenience really.
  //There cannot be a 0 axis, so this is invalid.
  //And we can't send 0 here.
  if(axis == 0 || value == 0)
    return;

  //To avoid clogging up the system with redundant information.
  
  if(lastSent[axis] == value)
    return;

  if(value == 255){
    value -= 1;
  }
  
  MAX.write((byte)0);
  delay(1);
  MAX.write(axis);
  delay(1);
  MAX.write(value);
  
  lastSent[axis] = value;
}

