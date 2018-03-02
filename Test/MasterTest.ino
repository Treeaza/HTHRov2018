#include <SoftwareSerial.h>

#define SSerialRX 10
#define SSerialTX 11
#define SSerialTxControl 3

#define RS485Transmit HIGH
#define RS485Receive LOW

#define LED 6
#define Pot 5

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Transmit);
  RS485Serial.begin(4800);
}

void loop(){
  int potRead = analogRead(Pot);
  byte mapped = map(potRead, 0, 1023, 0, 255);
  analogWrite(LED, mapped);
  RS485Serial.write(mapped);
  delay(50);
}
