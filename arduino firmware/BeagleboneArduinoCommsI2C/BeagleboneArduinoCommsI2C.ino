// Method Personal Food Computer
// This is the primary sketch for syncing the Arduino with the Beaglebone

#include <Wire.h>


#define NUM_ARGS 4
#define INVERTED_CHANNEL 5 // one channel is inverted due to faulty board

String inString[NUM_ARGS];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
byte messageIdx = 0;



byte pins[] = {3,4,5,6,7,8};

void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output

  for (byte i=0; i < NUM_ARGS; i++){
    inString[i].reserve(200);
  }

  LEDsetup();
  
  Serial.println("ready.");
}

void loop() {
  delay(10);
 
  if (stringComplete){
    if (inString[0] == "w"){
      write_output();
    } else if (inString[0] == "r"){
      read_register();
    } else if (inString[0] == "g"){
      write_register();
    } else {
      Serial.println("Unknown function code at inString[0]");
    }
    clearInString();
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
//  Serial.println("receiving data...");
  while (Wire.available()) { // loop through all but the last
    char inChar = Wire.read(); // receive byte as a character
//    Serial.print(inChar);         // print the character

    if (inChar == '*') {
        messageIdx = 0;
      } else if (inChar == '_') {
          messageIdx = messageIdx+1;
      } else {
        // add it to the inputString:
        inString[messageIdx] += inChar; 
      }
      
      if (inChar == '^'){
        messageIdx = 0;
        stringComplete = true;
      }
  }
  
//  Serial.println();  

  
         
}




void write_output(){
  byte pin = (byte)inString[1].toInt();
  byte val = (byte)inString[2].toInt();

  pinMode(pin, OUTPUT);
  if (pin < 13) {
    if (pin != INVERTED_CHANNEL){
      val = 255-val;
    }
    analogWrite(pin, val);
  } else {
    if (val > 0) { val = 1; };
    digitalWrite(pin, val);
  }

//  Serial.print("set ");
//  Serial.print(pin);
//  Serial.print(" to ");
//  Serial.println(val);
}


void read_register(){
  byte stringSize = inString[1].length()+1;
  char cString[stringSize];
  inString[1].toCharArray(cString, stringSize);  
  byte address = strtol(cString, NULL, 16);

  stringSize = inString[2].length()+1;
  cString[stringSize];
  inString[2].toCharArray(cString, stringSize);  
  byte reg = strtol(cString, NULL, 16);
  
//  Serial.println(address);
//  Serial.println(reg);
}

void write_register(){
  byte stringSize = inString[1].length()+1;
  char cString[stringSize];
  inString[1].toCharArray(cString, stringSize);  
  byte address = strtol(cString, NULL, 16);

  stringSize = inString[2].length()+1;
  cString[stringSize];
  inString[2].toCharArray(cString, stringSize);  
  byte reg = strtol(cString, NULL, 16);

  stringSize = inString[3].length()+1;
  cString[stringSize];
  inString[3].toCharArray(cString, stringSize);  
  byte val = strtol(cString, NULL, 16);
  
//  Serial.println(address);
//  Serial.println(reg);
//  Serial.println(val);
}

void clearInString(){
  // clear the string:
  for (byte i=0; i < NUM_ARGS; i++){
    inString[i] = "";
  }
  stringComplete = false;
}

void LEDsetup(){
    for (byte i=0; i < sizeof(pins); i++){
    byte off = 255;
    if (pins[i] == INVERTED_CHANNEL){
        off = 255-off;
      }
    pinMode(pins[i], OUTPUT);
    analogWrite(pins[i], off);
  }
}
