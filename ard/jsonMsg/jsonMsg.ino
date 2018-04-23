#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define ENABLE 7
#define msgDelay 1500

SoftwareSerial rs485(8,9);
byte userInput;
byte rx;
int msgNum=0;
byte rxBuf[200];
int index=0;
byte DEVICE=0;
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();


void programID(){
  EEPROM.write(0,1);
}

void readID(){
  DEVICE=EEPROM.read(0);
  if(DEVICE==0){
    Serial.println("FAILED TO READ ID");
    while(true){
      delay(1000);
    }
  }
  Serial.print("ID: ");
  Serial.println(DEVICE);
}

void setup() {
  Serial.begin(115200);
  programID();
  readID();
  Serial.println("Starting");
  rs485.begin(9600);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(1000);
  digitalWrite(13,LOW);
  pinMode(ENABLE,OUTPUT);
  digitalWrite(ENABLE,LOW);
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
  
}
