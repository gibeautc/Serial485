//sudo chmod a+rw /dev/ttyUSB1
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define ENABLE 6   //Used to switch rs485 module from tx/rx
#define RED 5
#define GREEN 4
#define BLUE 3
#define DEBUGRATE 115200
#define RS485RATE 38400
//may have to change these
#define SoilSensor A1
#define VALVE 2

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
SoftwareSerial rs485(8,7);
byte userInput;
byte rx;
int msgNum=0;
byte rxBuf[200];
int index=0;
byte DEVICE=0;
unsigned long lastContact=millis();
unsigned long lastAct=millis();
int adcBuf[20];
int adcIndex=0;
bool readyForAve=false;
bool valveTimeMode=true;  //if true, we will turn on water for specific time, then wait, and read again
                          //if false, we will wait for reading to determine if water should be turned off
bool AUTO=true;           //if true, uC is making own desisions about valve on/off
int threshold=500;        //no idea what this number will actually be yet
unsigned long lastValveTime;
unsigned long lastSoilTime=millis();




/*
 * Used to program a new ID into EEPROM, currently only using single byte address 0
 * Do not use 0 or 255
 */
void programID(){
  EEPROM.write(0,1);
}

/*
 * pulls current ID out of EEPROM. a 0 or 255 is concidered an error and will block
 */
void readID(){
  DEVICE=EEPROM.read(0);
  if(DEVICE==0 || DEVICE==255){
    Serial.println("FAILED TO READ ID");
    while(true){
      blinkSlow(RED);
    }
  }
  Serial.print("ID: ");
  Serial.println(DEVICE);
}


/*
 * Analize the current contents of the adcBuf. only does so if we have not checked in 5 minutes
 */
void checkSoil(){
  Serial.println("Analizing Soil Data");
  //only going to check if we have not turned on or off in the last 5 min
  if(millis()-lastValveTime>1000*60*5){
    unsigned long ave;
    for(int x=0;x<20;x++){
      ave+=adcBuf[x];
    }
    ave=ave/20;
    unsigned long t=millis();

      if(digitalRead(VALVE)){
        //if we are on
        if(valveTimeMode){
          //time mode, and we already waited 5 min
          digitalWrite(VALVE,LOW);
          lastValveTime=millis();
        }
        else{
          //threshold mode
          if(ave>threshold){
            digitalWrite(VALVE,LOW);
            lastValveTime=millis();
          }
        }  
      }
      else{
        //we are currently off
        if(ave<threshold){
          digitalWrite(VALVE,HIGH);
          lastValveTime=millis();
        }
      } 
    }
}

/*
 * takes raw reading from ADC, Takes 10 measurements 10ms appart and averages them 
 * could include throwing out max and min values
 */
void takeReading(){
  Serial.println("Taking Reading");
  if(adcIndex==19){readyForAve=true;}//once the buffer is full the first time, we are ready to analize
  unsigned long tempVal;
  for(int x=0;x<10;x++){
    tempVal+=analogRead(SoilSensor);
    delay(10);
  }
  adcBuf[adcIndex]=tempVal/10;
  adcIndex++;
  if(adcIndex==20){adcIndex=0;}
  if(readyForAve){
    checkSoil();
  }
}

/*
 * LED functions
 */
void blinkSlow(int pin){
  digitalWrite(pin,LOW);
  delay(250);
  digitalWrite(pin,HIGH);
  delay(500);
}
void blinkFast(int pin){
  digitalWrite(pin,LOW);
  delay(50);
  digitalWrite(pin,HIGH);
  delay(50);
}
void blinkVar(int pin,int d){
  digitalWrite(pin,LOW);
  delay(d);
  digitalWrite(pin,HIGH);
}

/*
 * Sends the init message to Master. This is used on startup, and will also be send if we have not 
 * been talked to for a certain amount of time
 */
void sendInit(){
  Serial.println("Sending init Request");
  root["id"]=DEVICE;
  root["type"]="init";
  root["status"]="";
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
  lastContact=lastContact+3000;
}


/*
 * Sends status update to Master after it has been requested
 */
void sendUpdate(){
  root["id"]=DEVICE;
  root["status"]="Online";
  root["type"]="update";
  root["mode"]=AUTO;
  if(readyForAve){
    if(adcIndex>1){root["soil_reading"]=adcBuf[adcIndex-1];}//only sending back most 
    else{root["soil_reading"]=adcBuf[19];}//only sending back most   
  }
  root["threshold"]=threshold;  

  //at current count, we are around 57 char in length
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
  lastContact=millis();
}

void checkRx(){
  while(rs485.available()){
    lastAct=millis();
    rx=rs485.read();
    if(rx==2){
      //blinkFast(RED);
      //start of message, so clear buffer
      memset(rxBuf,0,sizeof(rxBuf));
      index=0;
    }
    else if(rx==3){
      //end of messgae, we should have the whole thing now
      blinkFast(BLUE);
      checkMsg();
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

void checkMsg(){
  StaticJsonBuffer<200> jsonRxBuffer;
  JsonObject& rxRoot=jsonRxBuffer.parseObject((char*)rxBuf);

  // Test if parsing succeeds.
  if (!rxRoot.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  //check if its addressed to us, and a status request
  if(rxRoot["id"]==DEVICE && rxRoot["type"]=="status"){
    Serial.println("Request for status");
    sendUpdate();
    blinkFast(GREEN);
    return;
  }

  //if it was not a correct confirm, but it was to me, then show error, other wise just retrun peacefully
  if(rxRoot["device"]==DEVICE){
    Serial.print("Unknown Message for us");
    rxRoot.prettyPrintTo(Serial);
    Serial.println("");  
    blinkFast(RED);
  }
  return;
}


void setup() {
  Serial.begin(DEBUGRATE);
  readID();
  Serial.println("Starting");
  rs485.begin(RS485RATE);
  pinMode(ENABLE,OUTPUT);
  pinMode(RED,OUTPUT);    //setup LEDs as OUTPUT
  pinMode(BLUE,OUTPUT);
  pinMode(GREEN,OUTPUT);
  digitalWrite(ENABLE,LOW);
  
  blinkFast(RED);
  blinkFast(BLUE);
  blinkFast(GREEN);
}

void loop() {
  checkRx();
  unsigned long t=millis();
  if(t-lastContact>20000){
    AUTO=true;
    blinkSlow(RED);
    }//if we have not heard from master, go back to automode
  if(t-lastContact>20000 && t-lastAct>3000){sendInit();}//No recent contact, and line not busy, send init
  if(t-lastSoilTime>10000){
    takeReading();
    lastSoilTime=millis();
    }
}
