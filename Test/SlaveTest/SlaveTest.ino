#include <SoftwareSerial.h>

#define SSerialRX 10
#define SSerialTX 11
#define SSerialTxControl 3

#define RS485Transmit HIGH
#define RS485Receive LOW

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void setup(){
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);
  RS485Serial.begin(4800);
  Serial.begin(9600);
}

void loop(){
  if(RS485Serial.available()){
    byte r;
    while(RS485Serial.available()){
      r = RS485Serial.read();
      if(r == 0){
        process(RS485Serial.read(), RS485Serial.read());
      }
      //Serial.println(r);
    }
    Serial.println("End of backlog");
  }
  delay(5);
}

void process(byte a, byte b){
  if(a == 0 || b == 0 || b > 7)
    return;
  Serial.println(b);
  Serial.println(a);
}

