#include <SoftwareSerial.h>
#include <XBOXUSB.h>
#include <SPI.h>

#define CHANNELLR 1
#define CHANNELFB 2
#define CHANNELUD 3
#define CHANNELROTATION 4
#define CHANNELPITCH 5
#define CHANNELCLAWONE 6
#define CHANNELAUTOLEVEL 7
#define CHANNELCLAWTWO 8

#define MAXRXPin 5
#define MAXTXPin 6
#define MAXControl 3

#define MAXTX HIGH
#define MAXRX LOW

#define CHANNELS 8
#define LASTSENTRESETCOUNT 500

#define STATUSLEDPIN 4

USB Usb;
XBOXUSB Xbox(&Usb);

SoftwareSerial MAX(MAXRXPin, MAXTXPin);

bool clawMode = true;

void setup(){
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
  //Does...something with the shield? I've yet to work out exactly what, but it's important.
  Usb.Task();
  if(Xbox.Xbox360Connected) {
    
    digitalWrite(STATUSLEDPIN, HIGH);
    
    lastSentResetCounter--;
    if(lastSentResetCounter <= 0){
      for(int i = 0; i < CHANNELS; i++){
        lastSent[i] = 0;
      }
      lastSentResetCounter = LASTSENTRESETCOUNT;
    }
    
    //This could be very nicely generalized but I'm not a very nice general.

    //Jack wants rotation on this one.
    readAndSendAnalogHat(LeftHatX, CHANNELROTATION);
    //I'm told this should run up/down.
    readAndSendAnalogHat(LeftHatY, CHANNELUD);
    //Right stick is all planar controls.
    readAndSendAnalogHat(RightHatX, CHANNELLR);
    readAndSendAnalogHat(RightHatY, CHANNELFB);
   
    readAndSendSubtractiveTriggers(claw? CHANNELCLAWONE: CHANNELCLAWTWO);

    //Change claw mode.
    if(Xbox.getButtonClick(A)){
      claw = !claw;
    }
    setClawModeDisplay(claw);

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
  int value = Xbox.getAnalogHat(a);
  if(value > 8000 || value < -8000){
    byte mapped = map(value, -32768, 32768, 1, 254);
    sendCommand(channel, mapped);
  }else{
    sendCommand(channel, 128);
  }
}

void readAndSendSubtractiveTriggers(byte channel){
  byte l = Xbox.getButtonPress(L2);
  byte r = Xbox.getButtonPress(R2);
  l = map(l, 0, 255, 0, 127);
  r = map(r, 0, 255, 0, 127);
  byte ret = (128 + r - l);
  sendCommand(channel, ret);
}

void sendCommand (byte axis, byte value) {
  if(axis == 0 || value == 0)
    return;
  
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

