/*************************************************
OSEPP Real-time clock and microSD card example

Use the DS1307 RTC for accurate time keeping and log timestamps, 
sensor readings and other data with the microSD. 

How to wire:
Date and time functions using a DS1307 RTC connected via I2C and Wire lib
SD card attached to SPI bus as follows:
-MOSI - pin 11
-MISO - pin 12
-CLK -  pin 13
-CS  -  pin 4

   Copyright (C) 2016  Daniel Jay

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*************************************************/
 
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Servo.h>
#include "RTClib.h"
#include <HX711_ADC.h>
#include <HX711.h>
#define DOUT  3
#define CLK  2
#define LED_PIN 9
#define maxReading 20.0

#define PWMmax_R 1100
#define PWMmax_F 1900
#define PWMstall 1500
#define commonThrustPower 75
#define thrustTime 5000
#define loadCellFrequency 10 //in Hz
#define thresholdForceChangePerSecond 2.0 //if the rate of change in force deceted by the load cell is more than this value, motors will engage 

byte esc1pin = 5;
byte esc2pin = 6;
Servo ESC1;
Servo ESC2;

const int forwardPWM = PWMstall + ((PWMmax_F - PWMstall) * commonThrustPower / 100);

HX711 scale;

RTC_DS1307 RTC;

float calibration_factor = 20000.21; //-7050 worked for my 440lb max scale setup
const int chipSelect = 4;

void setup () {
  Serial.begin(57600);
  ESC1.attach(esc1pin);
  ESC2.attach(esc2pin);
  stopThrust();



  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  Wire.begin();
  RTC.begin();
 
  //if (! RTC.isrunning()) {
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  //}
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  scale.begin(DOUT, CLK);
  
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average();

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  digitalWrite(LED_PIN, LOW);
  delay(5000);
}


 
void loop () {
  unsigned long initT = millis();  

  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("File not available");
    return;
  }
  
  while (millis() < initT + 3600000) { //code will continuously take data for one hour
    scale.set_scale(calibration_factor);

    float reading = scale.get_units();
    int analogVal = map(abs(reading), 0, maxReading, 0, 255);

    analogWrite(LED_PIN, analogVal);
    
    DateTime now = RTC.now();
    
    String dataString = "";
    dataString = String(now.hour() +  now.minute() + now.second()) + "," + String(reading, 5);
    dataFile.println(dataString);
    
    delay(1000 / loadCellFrequency); //5 Hz readings
    if (abs(reading - scale.get_units()) > ((double)thresholdForceChangePerSecond / loadCellFrequency)) {
      forwardThrust();
      delay(thrustTime);
      stopThrust();
    }
    
  }
  dataFile.close();
  digitalWrite(LED_PIN, LOW);

  while(1) {
    
  }
}

void motorInput(int one, int two) {
  ESC1.writeMicroseconds(one);
  ESC2.writeMicroseconds(two);
}

void stopThrust() {
  motorInput(PWMstall, PWMstall);
}

void forwardThrust() {
  motorInput(forwardPWM, forwardPWM);
}
