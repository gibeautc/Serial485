#include <SoftwareSerial.h>
#define re 7
SoftwareSerial rs485(8,9);
byte userInput;
byte rx;
void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
  rs485.begin(9600);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(1000);
  digitalWrite(13,LOW);

  pinMode(re,OUTPUT);
  digitalWrite(re,LOW);
}

void userEcho(){
  while(true){
    if(Serial.available()){
      userInput=byte(Serial.read());
    }
    if(userInput!=0){
      Serial.println("UserInput");
      Serial.println(userInput);
      Serial.write(userInput);
      Serial.write('\n');
      rs485.write(userInput);
      userInput=0;
    }
    if(rs485.available()){
      rx=rs485.read();
      Serial.println("Rx byte:");
      Serial.println(rx);
      Serial.write(rx);
      Serial.write('\n');
    }  
  }
}


void autoEcho(){
  rx=0;
  byte newRx;
  unsigned long t;
  while (rx<255){
    //Serial.print("Sending: ");
    //Serial.println(rx);
    rs485.write(rx);
    t=millis();
    while(rs485.available()==false){
      if (millis()-t>1000){
        //Serial.println("TIMEOUT");
        break;
      }
    }
    newRx=rs485.read();
    if(rx==newRx){
       //Serial.println("Got number back");
       rx++;
     }
     //else{
       //Serial.print("Nope");
       //Serial.println(newRx);
     //}
  }
}

void loop() {
  unsigned long start=millis();
  autoEcho();
  Serial.print("Time: ");
  Serial.println(millis()-start);
  while(true){}
}
