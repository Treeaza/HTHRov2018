#include <SoftwareSerial.h>

#define SSerialRX 10
#define SSerialTX 11
#define SSerialTxControl 3

#define RS485Transmit HIGH
#define RS485Receive LOW

#define LED 6
#define Available 2

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(Available, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);
  RS485Serial.begin(4800);
}
int availableTime = 0;
void loop(){
  if(RS485Serial.available()){
    availableTime = 100;
    byte r = RS485Serial.read();
    analogWrite(LED, r);
  }
  availableTime--;
  digitalWrite(Available, availableTime > 0);
  delay(10);
}

