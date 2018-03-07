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

[TODO]

*/

#include <SoftwareSerial.h>
#include <XBOXUSB.h>
#include <SPI.h>

//Pins for connecting MAX485 to.
#define MAXRX 5
#define MAXTX 6
#define MAXControl 3

//Some constants to make serial direction control easier.
#define MAXTX HIGH
#define MAXRX LOW

//USB object to connect the shield, then use is to connect XBOX controller.
USB Usb;
XBOXUSB Xbox(&Usb);

//Actual serial object.
SoftwareSerial MAX(MAXRX, MAXTX);

void setup(){
  //Set 485 to transmit. We never actually need to recieve, but it's nice to have the option.
  pinMode(MAXControl, OUTPUT);
  digitalWrite(MAXControl, MAXTX);
  //Start the connection. Baud rate doesn't really matter that much here.
  MAX.begin(4800);
}

void loop(){
  //Does...something with the shield? I've yet to work out exactly what, but it's important.
  Usb.Task();
  if(Xbox.Xbox360Connected) {
    
  }
}

