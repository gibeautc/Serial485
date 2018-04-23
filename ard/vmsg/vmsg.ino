#include <EEPROM.h>
#include <SoftwareSerial.h>
#define ENABLE 7



//Commands 
#define cmdStatus 0x01
#define cmdConfirm 0x02




SoftwareSerial rs485(8,9);
unsigned long timer=millis();
byte DEVICE[8];
int msgDelay=0;


struct Msg{
  byte Cmd;
  byte Len;
  byte Pt1[2];
  byte Pt2[2];
  byte Status;
  byte LstCmd;
  byte* Data;
};



//only calling this on a new deivce or if we want to change its device ID
void programID(){
  //new id
  byte newID[]={1,1,2,2,3,3,4,4};
  for(int x=0;x<8;x++){
    EEPROM.write(x,newID[x]);  
  }
  
}

void readID(){
  for(int x=0;x<8;x++){
    DEVICE[x]=EEPROM.read(x);  
  }
  if(DEVICE[0]==0){
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
  readID();
  Serial.println("Starting");

  //setup r485 and to rx 
  pinMode(ENABLE,OUTPUT);
  digitalWrite(ENABLE,LOW);
  rs485.begin(9600);
  
}


void getMsg(){
  //read the header info
  Msg msg;
  msg.Cmd=rs485.read();
  msg.Len=rs485.read();
  msg.Pt1[0]=rs485.read();
  msg.Pt1[1]=rs485.read();
  msg.Pt2[0]=rs485.read();
  msg.Pt2[1]=rs485.read();
  msg.Status=rs485.read();
  msg.LstCmd=rs485.read();
  byte data[msg.Len];
  if(msg.Len>0){
    //we have data
    if(rs485.available()<msg.Len){
      Serial.println("not enought data in Rx buffer!!!")
      //not really sure what I should do here
      return
    }
    for(int x=0;x<msg.Len;x++){
      data[x]=  
    }
  }

  
    
}

void loop() {
  if(rs485.available()>7){
    getMsg()
  }

  if(millis()-timer>2000){
    Serial.print("Tx: ");
    if(DEVICE==1){
      Serial.println("A");  
    }
    if(DEVICE==2){
      Serial.println("B");  
    }
    digitalWrite(ENABLE,HIGH);
    if(DEVICE==1){
      rs485.write('A');  
    }
    if(DEVICE==2){
      rs485.write('B');
    }
    
    digitalWrite(ENABLE,LOW);
    timer=millis();
  }

}
