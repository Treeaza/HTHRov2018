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
}
void loop() {
  Usb.Task();
  if (Xbox.Xbox360Connected) {
    if (Xbox.getButtonPress(L2) || Xbox.getButtonPress(R2)) {
      Xbox.setRumbleOn(Xbox.getButtonPress(L2), Xbox.getButtonPress(R2));
    } else
      Xbox.setRumbleOn(0, 0);
    
    if (Xbox.getButtonClick(UP)) {
      Xbox.setLedOn(LED1);
    }
    if (Xbox.getButtonClick(DOWN)) {
      Xbox.setLedOn(LED4);
    }
    if (Xbox.getButtonClick(LEFT)) {
      Xbox.setLedOn(LED3);
    }
    if (Xbox.getButtonClick(RIGHT)) {
      Xbox.setLedOn(LED2);
    }

    if (Xbox.getButtonClick(START)) {
      Xbox.setLedMode(ALTERNATING);
    }
    if (Xbox.getButtonClick(BACK)) {
      Xbox.setLedBlink(ALL);
    }
    if (Xbox.getButtonClick(XBOX)) {
      Xbox.setLedMode(ROTATING);
    }
  }
  delay(1);
}
