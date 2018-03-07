#include <SoftwareSerial.h>
#include <XBOXUSB.h>
#include <SPI.h>

USB Usb;
XBOXUSB Xbox(&Usb);

#define SSerialRX 5
#define SSerialTX 6
#define SSerialTxControl 3

#define RS485Transmit HIGH
#define RS485Receive LOW

#define LED 6

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Transmit);
  RS485Serial.begin(4800);
  if (Usb.Init() == -1) {
    while (1);
  }
}

void loop(){
  Usb.Task();
  if (Xbox.Xbox360Connected) {
    Xbox.setLedOn(LED4);
    if (Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
      Xbox.setLedOn(LED3);
      int i = Xbox.getAnalogHat(LeftHatY);
      byte mapped = map(i, -32768, 32768, 0, 255);
      RS485Serial.write(mapped);
    }else{
      RS485Serial.write(128);
      Xbox.setAllOff();
    }
  }else{
    RS485Serial.write(128);
  }
  delay(1);
}
