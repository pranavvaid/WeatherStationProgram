/*
 * This is the program for my live weather station project
 * Its hardware, design process, and more information can be found at MY BLOG: https://makerprojectsblog.wordpress.com/
 */




#include <Wire.h>
#include "LedControl.h"
#include "SparkFunHTU21D.h"
HTU21D myHumidity;

const int startHour = 16; //Use 24 hour clock
const int startMin = 10;

int threshold = 53;
unsigned int updateSpeed = 8;
unsigned int switches = 0;  
unsigned int delaySpeed = 1300;
unsigned int offTime = 4100;
unsigned long minutesSinceStart = 0;
unsigned long secondsSinceStart = 0;
unsigned long millisCounter = 0;
unsigned long lastReading = 0;
int lowVal;
int highVal;
int light;
int firstVal;
int motion = 3;
int currentHour = startHour;
int currentMin = startMin;
int prevMin = startMin;
boolean printTemperature = true;
boolean powerSaveMode = false;
boolean enableOff = false;
String temperature;
String humidity;

LedControl lc = LedControl(12,11,10,1); 

void setup() {
  pinMode(A0,OUTPUT);
  delay(10);
  lc.shutdown(0,false);   /* The MAX72XX is in power-saving mode on startup, we have to do a wakeup call */
  lc.setIntensity(0,1);   /* Set the brightness to a medium values */
  lc.clearDisplay(0);     /* and clear the display */
  temperature = Temp();
  humidity = Humd();
  light = readLight();
  lowVal = light;
  highVal = light;
  firstVal = light;
  if(light>420){motion = 5;}
  else if(light>300){motion = 4;}
  else if(light>150){motion = 3;}
  else if(light<=150){motion = 2;}
}


void loop() {
  inTime(22,30,5,30);
  light = readLight();
  if(light<lowVal){lowVal = light;}
  if(light>highVal){highVal = light;}
  if(switches>= updateSpeed){
    enableOff = false;
    if((lowVal>(firstVal-motion)) && (highVal<(firstVal+motion))){
      enableOff = true;
    }
    else{firstVal = light; highVal = light; lowVal = light;}
    if(light>420){motion = 5;}
    else if(light>300){motion = 4;}
    else if(light>150){motion = 3;}
    else if(light<=150){motion = 2;}
    
    if(currentHour == 11 && currentMin<30){threshold = 410;}
    else if(currentHour == 11 && currentMin>30){threshold = 423;}
    else if(currentHour == 12 && currentMin<30){threshold = 455;}
    else if(currentHour == 12 && currentMin>30){threshold = 530;}
    else if(currentHour == 13 && currentMin<30){threshold = 335;}
    else if(currentHour == 13 && currentMin>30){threshold = 335;}
    else if(currentHour == 14 && currentMin<30){threshold = 340;}
    else if(currentHour == 14 && currentMin>30){threshold = 310;}
    else if(currentHour == 15 && currentMin<30){threshold = 285;}
    else if(currentHour == 15 && currentMin<40){threshold = 215;}
    else if(currentHour == 15 && currentMin<50){threshold = 170;}
    else if(currentHour == 15 && currentMin>50){threshold = 135;}
    else if(currentHour == 16 && currentMin<30){threshold = 87;}
    else if(currentHour == 16 && currentMin>30){threshold = 56;}
    else{threshold = 43;}

    switches = 0;
    temperature = Temp();
    humidity = Humd();
  }
  if(light<threshold){
    lc.shutdown(0,true);
    inTime(22,30,5,30);
    delay(12000);
    inTime(22,30,5,30);
    switches++;
  }
  else if(inTime(22,45,5,30)){  //Only time when time actually matters
    lc.shutdown(0,true);
    delay(58000);    // 58 seconds
    inTime(22,30,5,30);
    if(switches<updateSpeed-2){
      switches++;
    }
  }
  else if(enableOff == true){
    lc.shutdown(0,true);
    for(int i = 0; i<10;i++){
      light = readLight();
      if(light<lowVal){lowVal = light;}
      if(light>highVal){highVal = light;}
      delay(500);
      inTime(22,30,5,30);
    }
    inTime(22,30,5,30);
    enableOff = false;
    if(lowVal>(firstVal-motion) && highVal<(firstVal+motion)){
      enableOff = true;
    }
    else{firstVal = light; lowVal = light; highVal = light;}
    if(switches<updateSpeed-2){
      switches++;
    }
    if(light>420){motion = 5;}
    else if(light>300){motion = 4;}
    else if(light>150){motion = 3;}
    else if(light<=150){motion = 2;}
  }
  else{
    lc.shutdown(0,false);
    int brightness = map(light, threshold+75, 875, 0, 3);
    brightness = constrain(brightness, 1, 3);
    if(powerSaveMode == true){
      brightness = 1;
      delaySpeed = 1100;
      offTime = 7500;
    }
    else{
      delaySpeed = 1300;
      offTime = 4100;
    }
    lc.setIntensity(0,brightness);
    if(printTemperature){
      printOnLED(temperature);
      printF(0);
      delay(delaySpeed);
      inTime(22,30,5,30);
      lc.shutdown(0,true);
      for(int i = 0; i<4; i++){
        inTime(22,30,5,30);
        delay(offTime/4);
        light = readLight();
        if(light<lowVal){lowVal = light;}
        if(light>highVal){highVal = light;}
      } 
      inTime(22,30,5,30);
      delay(5);
      lc.shutdown(0,false);
      printOnLED(humidity);
      printH(0);
      delay(delaySpeed); 
      inTime(22,30,5,30);
      float battery = findVoltage();
      if(battery<20){
        powerSaveMode = true;
        printVoltage(convertToString(battery));
        delay(1500);
        inTime(22,30,5,30);
      }
      else{powerSaveMode = false;}
      lc.shutdown(0,true);
      delay(10);
      light = readLight();
      if(light<lowVal){lowVal = light;}
      if(light>highVal){highVal = light;}
      for(int i = 0; i<4; i++){
        inTime(22,30,5,30);
        delay(offTime/4);
        light = readLight();
        if(light<lowVal){lowVal = light;}
        if(light>highVal){highVal = light;}
      } 
      inTime(22,30,5,30);
      switches++;
    }
  }
}



String Temp(){
  float tempC = myHumidity.readTemperature(); //Define tempC as the temperature readings (in celcius)
  delay(5);
  tempC = myHumidity.readTemperature(); 
  delay(5);
  tempC = myHumidity.readTemperature();
  if(tempC>99){tempC = myHumidity.readTemperature();}
  float tempF = convertCtoF(tempC);           //Define tempF as converting tempC to faherenheit
  tempF = constrain(tempF, 0, 99.99);
  String temp = String(int(tempF*100));
  return temp;
}


String Humd(){
  float humd = myHumidity.readHumidity();     //Define humd as the humidity readings
  delay(5);
  humd = myHumidity.readHumidity();
  delay(5);
  humd = myHumidity.readHumidity();
  if(humd>99){humd = myHumidity.readHumidity();}
  humd = constrain(humd,0,99.99);
  String humidit = String(int(humd*100));
  return humidit;
}

void printF(int digitF){
  lc.setChar(0,digitF, 'F', 0);
}


void printH(int digitH){
  lc.setChar(0,digitH, 'h', 0);
}


void printOnLED(String number){
  lc.clearDisplay(0);
  for(int i = 0; i<number.length(); i++){
    int decimal = 0;
    if(i == 1 || i == 5){
      decimal = 1;
    }
    lc.setDigit(0,number.length()-i-1, number[i]-'0',decimal); 
  }
}


float convertCtoF(float tempC){ //Function for converting Celcius to Fahrenheit
  return (1.8*tempC)+32; //Return the value of the conversion
}


int readLight(){
  digitalWrite(A0, HIGH);
  int lightVal = analogRead(A1);
  digitalWrite(A0,LOW);
  return lightVal;
}


void printVoltage(String number){
  lc.clearDisplay(0);
  for(int i = 0; i<number.length(); i++){
    int decimal = 0;
    lc.setDigit(0,number.length()-i-1, number[i]-'0',decimal); 
  }
  lc.setChar(0,0,'p',0);
  lc.setChar(0,1,' ',0);
}


float findVoltage(){
  unsigned int averageVolt = 0;
  for(int i = 0; i<15; i++){
    int value= analogRead(A2);
    float voltage = value;
    voltage = voltage/1024;
    voltage = voltage*3.3;
    voltage = voltage*2;
    voltage = voltage - 3.2;
    voltage = voltage*100;
    averageVolt = averageVolt + voltage;
    delay(1);
  }
  averageVolt = averageVolt/15;
  return averageVolt;
}


String convertToString(float number){
  String finalVoltage = String(int(number*100));
  return finalVoltage;
}

boolean inTime(int lowerHour, int lowerMin, int higherHour, int higherMin){//(lower time limit, higher time limit) USE 24 HOUR TIME EX: USE 18 TO SHOW 6PM
  millisCounter = millis()- lastReading;
  if(millis()>=4294950000){
    secondsSinceStart = secondsSinceStart + (millisCounter/1000);
    delay(25000);
    secondsSinceStart+=25;
    lastReading = millis() - (millisCounter%1000);
  }
  else if(millisCounter>=1000){
    secondsSinceStart = secondsSinceStart + (millisCounter/1000);
    lastReading = millis() - (millisCounter%1000);
  }
  minutesSinceStart = secondsSinceStart/60;
  currentMin = (((minutesSinceStart%60) + startMin) % 60);
  if(prevMin==59 && currentMin == 0){currentHour = ((currentHour+1)%24);}
  prevMin = currentMin;
  for(int i = lowerHour; i<=higherHour; i = (i+1)%24){
    if(currentHour == i){
      if(currentMin>=lowerMin || currentHour > lowerHour){
        if(currentMin<=higherMin || currentHour< higherHour){return true;}
      }
    }
  }
  return false;
}
