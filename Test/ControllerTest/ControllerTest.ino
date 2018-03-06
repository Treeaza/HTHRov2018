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
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nXBOX USB Library Started"));
}
void loop() {
  Usb.Task();
  if (Xbox.Xbox360Connected) {
    if (Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
      if (Xbox.getAnalogHat(LeftHatY) > 7500 || Xbox.getAnalogHat(LeftHatY) < -7500) {
        Serial.print(F("LeftHatY: "));
        int i = Xbox.getAnalogHat(LeftHatY);
        i = map(i, -32768, 32768, 0, 255);
        Serial.print(i);
        analogWrite(5, i);
        Serial.print("\t");
      }else {
        analogWrite(5, 128);
      }
      Serial.println();
    }
  }
  delay(1);
}
