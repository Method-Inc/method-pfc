// Method Personal Food Computer
// This sketch is for making the Arduino act on it's own without the Beaglebone. Useful for testing connectivity.

#include <Wire.h>

// Communication Protocol: [function][message]
// function: 
//    [w]: write_output
//    [r]: read_register
//    [g]: write_register
//    [s]: setModeTo
//    [m]: report current mode
//    [t]: setTime
// 
// message:
//    *w_[pin]_[value]^
//    *r[address]_[register]^
//    *g_[address]_[register]_[value]^
//    *m_[mode]^


#define NUM_ARGS 4
#define INVERTED_CHANNEL 5
String inString[NUM_ARGS];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
byte messageIdx = 0;

String currentMode;
byte currentTime[] = {0,0}; // {Hour, Minute}
unsigned long secondStart = 0;
int seconds = 0;

// Timer mode variables
byte daySchedule[] = {8,23};
byte nightSchedule[] = {23,8};
#define INITIAL_HOUR 9
#define INITIAL_MINUTE 30


byte pins[] = {3,4,5,6,7,8};

void(* resetFunc) (void) = 0;//declare reset function at address 0

void setup() {
  // initialize serial:
  Serial.begin(9600);
  Serial.println("Initializing...");
  // reserve 200 bytes for each input:
  for (byte i=0; i < NUM_ARGS; i++){
    inString[i].reserve(200);
  }

  LEDsetup();

  
  setModeTo("TIMER");
  
  Serial.println("Ready.");

  setTime(INITIAL_HOUR, INITIAL_MINUTE);
  
}

void loop() {
  serialEvent(); //call the function

  // global commands
  if (stringComplete){
    if (inString[0] == "m"){ // report mode
      Serial.print("Current mode: ");
      Serial.println(currentMode);
      clearInString();
    } else if (inString[0] == "s"){ // set mode
      setModeTo(inString[1]);
      clearInString();
    } else if (inString[0] == "t"){ // set clock time
      setTime((byte)inString[1].toInt(), (byte)inString[2].toInt());
      clearInString();
    }
  }
  
  if (currentMode == "TIMER") {
    loop_timerMode();
  } else if (currentMode == "MANUAL"){
    loop_manualMode();
  } else {
    Serial.println("This mode is not recognized. Please set to TIMER or MANUAL.");
    Serial.println("Setting mode to TIMER...");
    setModeTo("TIMER");
  }

  keepTime();
  
}


void loop_manualMode(){
  if (stringComplete){
    if (inString[0] == "w"){
      write_output();
    } else if (inString[0] == "r"){
      read_register();
    } else if (inString[0] == "g"){
      write_register();
    } else if (inString[0] == "s"){
      setModeTo(inString[1]);
    } else {
      Serial.println("Unknown function code at inString[0]");
    }
    clearInString();
  }

}

void loop_timerMode(){
  if (stringComplete){
      Serial.println("Must be set to manual mode to process commands. Send the command: *m_MANUAL^");
    clearInString();
  }

  // day or night?
  if (currentTime[0] > daySchedule[0] && currentTime[0] < daySchedule[1]){
    //day
    dayPhase();
  } else {
    nightPhase();
  }

  
}


/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    
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
      
    
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    
  }
}


void write_output(){
  byte pin = (byte)inString[1].toInt();
  byte val = (byte)inString[2].toInt();

  pinMode(pin, OUTPUT);
  if (pin < 13) {
    analogWrite(pin, val);
  } else {
    if (val > 0) { val = 1; };
    digitalWrite(pin, val);
  }

  Serial.print("set ");
  Serial.print(pin);
  Serial.print(" to ");
  Serial.println(val);
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
  
  Serial.println(address);
  Serial.println(reg);
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
  
  Serial.println(address);
  Serial.println(reg);
  Serial.println(val);
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

void setModeTo(String newMode){
  Serial.print("Setting mode to: ");
  Serial.println(newMode);
  currentMode = newMode;
}

void dayPhase(){
  byte actuatorPins[7] = {6,7,8,3,4,5,34};
  byte lightingValues[7] = {255,50,255,100,120,120, 1};   // {'FR', 'R', 'G', 'B', 'CW', 'WW', 'Fans'}

  // set lights
  for (byte i=0; i<sizeof(actuatorPins); i++){
    byte pin = actuatorPins[i];
    byte val = lightingValues[i];
    if (pin < 13) {
      if (pin != INVERTED_CHANNEL){
        val = 255-val;
      }
      analogWrite(pin, val);
    } else {
      if (val > 0) { val = 1; };
      digitalWrite(pin, val);
    }
  }
}

void nightPhase(){
  byte actuatorPins[7] = {6,7,8,3,4,5,34};
  byte lightingValues[7] = {0,0,0,0,0,0,1};   // {'FR', 'R', 'G', 'B', 'CW', 'WW', 'Fans'}

  // set lights
  for (byte i=0; i<sizeof(actuatorPins); i++){
    byte pin = actuatorPins[i];
    byte val = lightingValues[i];
    if (pin < 13) {
      if (pin != INVERTED_CHANNEL){
        val = 255-val;
      }
      analogWrite(pin, val);
    } else {
      if (val > 0) { val = 1; };
      digitalWrite(pin, val);
    }
  }
}

void setTime(byte hour, byte minute){
  if (hour < 24 && hour >= 0){
    currentTime[0] = hour;
  } else {
    Serial.println("Please enter a valid time: [Hour 0-23][Minute 0-59]");
    return;
  }
  if (minute < 60 && minute >= 0){
    currentTime[1] = minute;
  } else {
    Serial.println("Please enter a valid time: [Hour 0-23][Minute 0-59]");
    return;
  }

  currentTime[0] = hour;
  currentTime[1] = minute;

  Serial.print("New time: ");
  Serial.print(currentTime[0]);
  Serial.print(":");
  Serial.print(currentTime[1]);
  Serial.print(":");
  Serial.println(seconds);
}

void keepTime(){
  if (secondStart == 0){
    secondStart = millis();
  }

  if (millis() - secondStart >= 1000){ // 1 second has passed
    seconds++;
    secondStart = 0;
//    Serial.print("currentTime: ");
//    Serial.print(currentTime[0]);
//    Serial.print(":");
//    Serial.print(currentTime[1]);
//    Serial.print(":");
//    Serial.println(seconds);

    if (currentTime[0] == INITIAL_HOUR && currentTime[1] == INITIAL_MINUTE-1 && seconds == 60){
      // reset arduino at initial time
      resetFunc();
    }
  }

  if (seconds >= 60){
    seconds = 0;
    currentTime[1]++;
  }

  if (currentTime[1] >= 60){
    currentTime[1] = 0;
    currentTime[0]++;
  }

  if (currentTime[0] >= 24){
    currentTime[0] = 0;
  }

  

  
}
