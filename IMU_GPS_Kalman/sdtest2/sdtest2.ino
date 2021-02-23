/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/
#include <MPU9250.h>
#include <Adafruit_SSD1306.h>
#include "eeprom_utils.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#define calib false

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int chipSelect = 4;
MPU9250 mpu;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Wire.begin();
  while (!Serial) {}
  
  /*
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");
  */
  
  if (!mpu.setup(0x68)) { // change to your own address
    while(1) {
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
      delay(5000);
    }
  }

  display.begin(SSD1306_SWITCHCAPVCC , 0x3C);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.clearDisplay();

  if (calib) {
    display.print("Accel & Gyro");
    display.display();
    mpu.calibrateAccelGyro();
    display.setCursor(0, 0);
    display.clearDisplay();
    display.print("Mag");
    display.display();
    mpu.calibrateMag();
    saveCalibration();
    //mpu.printCalibration();
  }
  
  loadCalibration();
  
  display.setCursor(0, 0);
  display.clearDisplay();
  display.print("working");
  display.display();

  Serial.print("X");
  Serial.print(",");
  Serial.print("Y");
  Serial.print(",");
  Serial.println("Z");
}

void loop() {
/*
  // make a string for assembling the data to log:
  unsigned long initT = millis();
  String dataString = String(initT);
  while (millis() < initT + 1000) {}

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  }
  else {
    Serial.println("error opening datalog.txt");
  }
  */

  if (mpu.update()) {
    //mpu.printRollPitchYaw();
    Serial.print(mpu.getMagX(), 2);
    Serial.print(",");
    Serial.print(mpu.getMagY(), 2);
    Serial.print(",");
    Serial.println(mpu.getMagZ(), 2);
    delay(30);
  }
}
