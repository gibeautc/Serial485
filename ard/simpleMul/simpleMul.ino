#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define ENABLE 6   //Used to switch rs485 module from tx/rx
#define RED 5
#define GREEN 4
#define BLUE 3
#define DEBUGRATE 115200
#define RS485RATE 9600

SoftwareSerial rs485(8,7);
int msgDelay=1500;
unsigned long lastContact=millis();
unsigned long timer=millis();
byte DEVICE=0;
byte userInput;
byte rx;
int msgNum=0;
byte rxBuf[200];
int index=0;
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();


/*
Write a static number into EEPROM address 0
Only call on a new board, or if the address needs to be changed
*/
void programID(){
  EEPROM.write(0,2);
}



/*
Read device address from EEPROM address 0, currenly only a single 
byte
*/
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


/*
 * Some easy functions for blinking LED's on specified pins
 */
void blinkSlow(int pin){
  digitalWrite(pin,LOW);
  delay(250);
  digitalWrite(pin,HIGH);
  delay(250);
}

void blinkFast(int pin){
  digitalWrite(pin,LOW);
  delay(200);
  digitalWrite(pin,HIGH);
  delay(100);
}

void blinkVar(int pin,int d){
  digitalWrite(pin,LOW);
  delay(d);
  digitalWrite(pin,HIGH);
}


void setup() {
  Serial.begin(DEBUGRATE);  //used for debug
  pinMode(RED,OUTPUT);    //setup LEDs as OUTPUT
  pinMode(BLUE,OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(ENABLE,OUTPUT);   //set as Rx to start
  digitalWrite(ENABLE,LOW);
  readID();         //Get device ID from EEPROM
  
  Serial.println("Starting");
  rs485.begin(RS485RATE);
  serFlush();
  //blink all LEDs to show they are working, and that startup
  //has completed
  blinkSlow(RED);
  blinkSlow(BLUE);
  blinkSlow(GREEN);

  blinkFast(RED);
  blinkFast(BLUE);
  blinkFast(GREEN);
  serFlush();
}//End Setup

void serFlush(){
  byte trash;
  while(rs485.available()){
    trash=rs485.read();
  }
}

void Rx(){
  byte rx=0;
  Serial.print("Available: ");
  Serial.println(rs485.available());
  if(rs485.available()){
    delay(20);
    Serial.print("Rx: ");
    rx=rs485.read();
    blinkVar(BLUE,50);
    Serial.println(rx);
  }
  if(millis()-lastContact>30000){
    blinkSlow(RED);
  }

  if(rx==DEVICE){
    lastContact=millis();
    Serial.println("Thats for me!");
    digitalWrite(ENABLE,HIGH);
    rs485.write(rx);
    digitalWrite(ENABLE,LOW);
    blinkFast(GREEN);
  }
}

void Tx(){
  rs485.write('A');
  delay(1000);
}


void sendUpdate(){
  root["device"]=DEVICE;
  root["status"]="good";
  root["msgnum"]=msgNum;
  char msg[100];
  root.printTo((char*)msg, root.measureLength() + 1);
  Serial.print("Sending: ");
  Serial.print(strlen(msg));
  Serial.println(msg);
  
  digitalWrite(ENABLE,HIGH);
  rs485.write(0x02);
  for(int x=0;x<strlen(msg);x++){
    rs485.write(msg[x]);  
  }
  rs485.write(0x03);
  digitalWrite(ENABLE,LOW);
}


void checkConfirm(){
  StaticJsonBuffer<200> jsonRxBuffer;
  //Serial.println((char*)rxBuf);
  JsonObject& rxRoot=jsonRxBuffer.parseObject((char*)rxBuf);

  // Test if parsing succeeds.
  if (!rxRoot.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  if(rxRoot["device"]==DEVICE && rxRoot["msgnum"]==msgNum && rxRoot["status"]=="confirm"){
    Serial.println("Msg Confirmed");
    msgNum++;
    return;
  }

  //if it was not a correct confirm, but it was to me, then show error, other wise just retrun peacefully
  if(rxRoot["device"]==DEVICE){
    Serial.print("Confirm Failed:");
    rxRoot.prettyPrintTo(Serial);
    Serial.println("");  
  }
  return;
}

void checkRx(){
  
  while(rs485.available()){
    rx=rs485.read();
    if(rx==2){
      //start of message, so clear buffer
      memset(rxBuf,0,sizeof(rxBuf));
      index=0;
    }
    else if(rx==3){
      //end of messgae, we should have the whole thing now
      checkConfirm();
    }
    else{
      rxBuf[index]=rx;
      index++;
      if(index==200){
        index=0;
      }
    }
    
    
  }
}

void loop() {
  sendUpdate();
  delay(msgDelay);
  checkRx();
}//End main loop
