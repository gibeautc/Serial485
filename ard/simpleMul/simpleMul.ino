#include <EEPROM.h>
#include <SoftwareSerial.h>
#define ENABLE 6
#define LED 13

//not sure the colors are right yet
#define RED 3
#define GREEN 4
#define BLUE 5

SoftwareSerial rs485(8,7);
unsigned long timer=millis();
byte DEVICE=0;
int msgDelay=0;
void programID(){
  EEPROM.write(0,2);
}

void readID(){
  DEVICE=EEPROM.read(0);
  if(DEVICE<1||DEVICE>20){
    Serial.println("FAILED TO READ ID(Should be between 1 and 20");
    while(true){
      delay(1000);
    }
  }
  Serial.print("ID: ");
  Serial.println(DEVICE);
}

void blinkSlow(int pin){
  digitalWrite(pin,HIGH);
  delay(500);
  digitalWrite(pin,LOW);
}

void blinkFast(int pin){
  digitalWrite(pin,HIGH);
  delay(200);
  digitalWrite(pin,LOW);
}

void blinkVar(int pin,int d){
  digitalWrite(pin,HIGH);
  delay(d);
  digitalWrite(pin,LOW);
}


void setup() {
  Serial.begin(115200);
  pinMode(RED,OUTPUT);
  pinMode(BLUE,OUTPUT);
  pinMode(GREEN,OUTPUT);
  readID();
  msgDelay=1000*DEVICE;
  Serial.println("Starting");
  rs485.begin(9600);
  pinMode(ENABLE,OUTPUT);
  digitalWrite(ENABLE,LOW);


  blinkSlow(LED);
  blinkSlow(RED);
  blinkSlow(BLUE);
  blinkSlow(GREEN);
  
}

void loop() {
  byte rx=0;
  if(rs485.available()){
    delay(20);
    Serial.print("Rx: ");
    rx=rs485.read();
    blinkFast(LED);
    Serial.println(rx);
  }

  if(rx==DEVICE){
    Serial.println("Thats for me!");
    digitalWrite(ENABLE,HIGH);
    rs485.write(rx);
    digitalWrite(ENABLE,LOW);
  }
}
