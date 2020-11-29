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
    Serial.begin(9600);
   
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
   
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  String dataString = "Hello world";
  // if the file is available, write to it:
  if (dataFile) {
    while (1) {
      dataFile.println(dataString);
      Serial.println(dataString);
      delay(500);
    }
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  dataFile.close();
}
