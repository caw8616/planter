//4 Digit Display D2
//Digital Light Sensor I2C
//LED BAR D4
//MoistureSensor A0

#include <EEPROM.h>
#include <TimerOne.h>
#include <avr/pgmspace.h>
#include <TM1637.h>
#include <Digital_Light_TSL2561.h>
#include <Wire.h>
#include <Grove_LED_Bar.h>

//omega2-ctrl gpiomux set i2c gpio

#define ON 1
#define OFF 0

int8_t AcceptTime[] = {0x00, 0x00, 0x00, 0x00};
unsigned char ClockPoint = 1;
unsigned char Update;
unsigned char halfsecond = 0;
unsigned char second;
unsigned char minute = 0;
unsigned char hour = 0;
//unsigned char minute = 58;
//unsigned char hour = 23;

int8_t Timer[] = {0x00, 0x00, 0x00, 0x00};
unsigned char T_ClockPoint = 1;
unsigned char T_Update;
unsigned char t_halfsecond = 0;
unsigned char t_second;
unsigned char t_minute = 0;
unsigned char t_hour = 0;

#define CLK 2
#define DIO 3
TM1637 tm1637(CLK, DIO);

long lightFloor = 2000;
long lightCeiling = 10000;

Grove_LED_Bar LEDBar(5,4,0);
const int pinButton = 2;
const int led = 13;

int moisturePin = A0;    // select the input pin for the potentiometer
int moistureValue = 0;  // variable to store the value coming from the sensor

void setup() {
  
  Serial.begin(9600);

  tm1637.set();
  tm1637.init();
  Timer1.initialize(500000); 
  Timer1.attachInterrupt(AcceptTimingISR); 
  Wire.begin();
  TSL2561.init(); 
  LEDBar.begin();
  pinMode(led, OUTPUT);
}

void loop() {
     long level = TSL2561.readVisibleLux();
  if(Update == ON) {
    AcceptTimeUpdate();
    CheckAcceptTime(level);
    tm1637.display(Timer);
  }

  moistureValue = analogRead(moisturePin);    
  digitalWrite(led, HIGH);
  checkMoistureLevel(moistureValue);  

  
  printOutput(level, moistureValue);

//Every 1 Second
    delay(1000);
}

void CheckAcceptTime(long level){
  if((level > lightFloor) && (level < lightCeiling)) {
        ClockISR();
        ClockUpdate();
    }
}

//Checks to see how far away the Moisture level is to the midpoint at 475
void checkMoistureLevel(int moistureValue) {
    moistureValue = (moistureValue <= 475) ? moistureValue: (moistureValue-(moistureValue-475));
    int ledLevel = map(moistureValue,0,475,0,10);
    setBar(ledLevel);
}

//Sets LED Bar
void setBar(int bar) {
  for(int k=10; k > bar; k--) {
    LEDBar.setLed(k,0);
  }
  for(int i=1; i <= bar; i++) {
    LEDBar.setLed(i,1);
  }
}

//Timer for the 24 hour timer for reset
void AcceptTimingISR() {
  halfsecond++;
  Update=ON;
  if(halfsecond == 2) {
    second++;
    if(second == 60) {
      minute++;
      if(minute == 60) {
        hour++;
        if(hour == 24) {
          hour=0;
          resetClock();
        }
        minute = 0;
      } 
      second = 0;
    }
  halfsecond = 0;
  }
  ClockPoint = (~ClockPoint) & 0x01;
}

//update the Accept Time variable
void AcceptTimeUpdate(void) {
  if(ClockPoint) tm1637.point(POINT_ON);
  else tm1637.point(POINT_OFF);
  AcceptTime[0] = hour/10;
  AcceptTime[1]= hour%10; 
  AcceptTime[2] = minute/10;
  AcceptTime[3] = minute%10;
  Update=OFF;
}

//Timer for the time that the light has been at an acceptable level
void ClockISR() {
  t_second++;
  Update=ON;
    if(t_second == 60) {
      t_minute++;
      if(t_minute == 60) {
        t_hour++;
        if(t_hour == 24) hour=0;
        t_minute = 0;
      } 
      t_second = 0;
    }
  ClockPoint = (~ClockPoint) & 0x01;
}

//Update the clock for the light sensor
void ClockUpdate(void) {
  if(ClockPoint) tm1637.point(POINT_ON);
  else tm1637.point(POINT_OFF);
  Timer[0] = t_hour/10;
  Timer[1]  = t_hour%10; 
  Timer[2] = t_minute/10;
  Timer[3] = t_minute%10;
  Update=OFF;
}

//Resets both clocks, used when it reaches 24 hours
void resetClock() {
  halfsecond = 0;
  second = 0;
  minute = 0;
  hour = 0;
  t_halfsecond = 0;
  t_second = 0;
  t_minute = 0;
  t_hour = 0;
  Update = ON;
}

void printOutput(long level, int moistureValue) {
  Serial.print("Accept Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
  Serial.print("Clock Time: ");
  Serial.print(t_hour);
  Serial.print(":");
  Serial.print(t_minute);
  Serial.print(":");
  Serial.println(t_second);
  Serial.print("Digital Light Level: ");
  Serial.println(level);
  Serial.print("Moisture: " );                       
  Serial.println(moistureValue); 
  Serial.println();
}
