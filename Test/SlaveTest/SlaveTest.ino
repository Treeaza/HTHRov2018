#include <SoftwareSerial.h>

#define SSerialRX 10
#define SSerialTX 11
#define SSerialTxControl 3

#define RS485Transmit HIGH
#define RS485Receive LOW

#define LED 5

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);
  RS485Serial.begin(4800);
}

void loop(){
  if(RS485Serial.available()){
    byte r;
    while(RS485Serial.available()){
      r = RS485Serial.read();
    }
    analogWrite(LED, r);
  }
  delay(10);
}

