#include <SoftwareSerial.h>

#define SSerialRX 13
#define SSerialTX 12
#define SSerialTxControl 3

#define RS485Transmit HIGH
#define RS485Receive LOW

SoftwareSerial RS485Serial(SSerialRX, SSerialTX);

void setup(){
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);
  RS485Serial.begin(4800);
  Serial.begin(9600);
  Serial.println("Running");
}

void loop(){
  while(RS485Serial.available()){
    byte r = RS485Serial.read();
    if(r == 0){
      process(RS485Serial.read(), RS485Serial.read());
    }
  }
  Serial.println("End of backlog");
  delay(5);
}

void process(byte a, byte b){
  if(a == 0 || b == 0 || b > 8){
    return;
  }
  Serial.println(b);
  Serial.println(a);
}

