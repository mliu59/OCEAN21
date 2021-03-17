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

#include "RTClib.h"
 
RTC_DS1307 RTC;
const int chipSelect = 4;
 
void setup () {
    Serial.begin(57600);
   
    Wire.begin();
    RTC.begin();
 
  //if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  //}
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}


 
void loop () {
    DateTime now = RTC.now();
 
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
 
   
    // calculate a date which is 7 days and 30 seconds into the future
    //DateTime future (now.unixtime() + 7 * 86400L + 30);

    // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  //for (int analogPin = 0; analogPin < 3; analogPin++) {
    //int sensor = analogRead(analogPin);
    dataString = String(now.month()  + now.day() +  
    now.year() +  now.hour() +  now.minute() + 
    now.second());
    
    
    /*
    if (analogPin < 2) {
      dataString += ",";
    }
    */
  //}

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  

    delay(3000);
}
