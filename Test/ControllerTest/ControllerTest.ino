/*
 Example sketch for the Xbox 360 USB library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
 */

#include <XBOXUSB.h>
#include <SPI.h>

USB Usb;
XBOXUSB Xbox(&Usb);

void setup() {
  if (Usb.Init() == -1) {
    while (1); //halt
  }
  pinMode(5, OUTPUT);
}

void loop() {
  Usb.Task();
  if (Xbox.Xbox360Connected) {
    int i = Xbox.getAnalogHat(LeftHatY);
    i = map(i, -32768, 32768, 0, 255);
    if(i > 100){
      Xbox.setLedOn(LED1);
    }else {
        Xbox.setAllOff();
    }
    analogWrite(i, 5);
  }
  delay(5);
}
