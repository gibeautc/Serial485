#include <EEPROM.h>
#include <SoftwareSerial.h>



#define ENABLE 6   //Used to switch rs485 module from tx/rx
#define LED 13      //build in LED on board

//not sure the colors are right yet
#define RED 3
#define GREEN 4
#define BLUE 5

SoftwareSerial rs485(8,7);

unsigned long timer=millis();
byte DEVICE=0;

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
  Serial.begin(115200);  //used for debug
  pinMode(LED,OUTPUT);
  pinMode(RED,OUTPUT);    //setup LEDs as OUTPUT
  pinMode(BLUE,OUTPUT);
  pinMode(GREEN,OUTPUT);
  readID();         //Get device ID from EEPROM
  Serial.println("Starting");
  rs485.begin(9600);
  pinMode(ENABLE,OUTPUT);   //set as Rx to start
  digitalWrite(ENABLE,LOW);


  //blink all LEDs to show they are working, and that startup
  //has completed
  blinkSlow(LED);
  blinkSlow(RED);
  blinkSlow(BLUE);
  blinkSlow(GREEN);
  
}//End Setup

void loop() {
  byte rx=0;
  if(rs485.available()){
    delay(20);
    Serial.print("Rx: ");
    rx=rs485.read();
    blinkFast(LED);
    //blinkVar(RED,50);
    Serial.println(rx);
  }

  if(rx==DEVICE){
    Serial.println("Thats for me!");
    digitalWrite(ENABLE,HIGH);
    rs485.write(rx);
    digitalWrite(ENABLE,LOW);
    //blinkFast(GREEN);
  }
}//End main loop
