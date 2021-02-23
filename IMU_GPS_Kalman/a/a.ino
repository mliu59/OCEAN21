/*
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
*/
#include <MPU9250.h>
#include <Adafruit_SSD1306.h>
#include "eeprom_utils.h"
#include <EEPROM.h>
#include <EEWrap.h>
#include <SPI.h>
//#include <SD.h>
#include <Wire.h>
#include <BasicLinearAlgebra.h>

/*#include <TinyGPS.h>
#include <SoftwareSerial.h>
SoftwareSerial ss(8, 9);
*/
using namespace BLA;

#define calib true
#define IMU_READ_PERIOD_MS 50
#define GPS_READ_PERIOD_MS 1000

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define chipSelect 4
MPU9250 mpu;
//TinyGPS gps;

//unsigned long t0 = 0;
//unsigned long t1;
//unsigned long newGPSTime;
//float deltaT = 1.0; //in seconds

float u = 0;
float v = 0;

/*
Matrix<6> x0, x, x_;
Matrix<6, 6> A, P0, P_, P, Q;
Matrix<6, 6> I = {1, 0, 0, 0, 0, 0,
                  0, 1, 0, 0, 0, 0,
                  0, 0, 1, 0, 0, 0,
                  0, 0, 0, 1, 0, 0,
                  0, 0, 0, 0, 1, 0,
                  0, 0, 0, 0, 0, 1};
Matrix<4, 6> H = {1, 0, 0, 0, 0, 0,
                  0, 1, 0, 0, 0, 0,
                  0, 0, 0, 0, 1, 0,
                  0, 0, 0, 0, 0, 1};
Matrix<4, 4> R;
Matrix<6, 4> K;
Matrix<4> z;
Matrix<2> u;
Matrix<6, 2> B;

float lat, lon, aN, aE, disN, disE;
unsigned long age;

/*
struct Quaternion { float w, x, y, z; };

Quaternion ToQuaternion(float yaw, float pitch, float roll) { // yaw (Z), pitch (Y), roll (X) 
  // Abbreviations for the various angular functions
  double cy = cos(yaw * 0.5);
  double sy = sin(yaw * 0.5);
  double cp = cos(pitch * 0.5);
  double sp = sin(pitch * 0.5);
  double cr = cos(roll * 0.5);
  double sr = sin(roll * 0.5);
  Quaternion q;
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;
  return q;
}

Quaternion getQuaternion(MPU9250 a) {
  Quaternion q;
  q.w = a.getQuaternionW();
  q.x = a.getQuaternionX();
  q.y = a.getQuaternionY();
  q.z = a.getQuaternionZ();
  return q;
}
*/

Matrix<3, 3> toWorldFrame(double a, double b, double c) {
  Matrix<3, 3> forward = {cos(b)*cos(a), cos(c)*sin(a)+sin(c)*sin(b)*cos(a), sin(c)*cos(a)+cos(c)*sin(b)*cos(a), 
                          cos(b)*sin(a), cos(c)*cos(a)+sin(c)*sin(b)*sin(a), -sin(c)*cos(a)+cos(c)*sin(b)*sin(a),
                          -sin(b), sin(c)*cos(b), cos(c)*cos(b)};
  return forward.Inverse();
}

void setup() {
  
  //B.Fill(0);
  //u.Fill(0);
  
  // Open serial communications and wait for port to open:
  //Serial.begin(115200);
  Wire.begin();
  //ss.begin(9600);
  //while (!Serial) {}

  /*
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");
  */

  display.begin(SSD1306_SWITCHCAPVCC , 0x3C);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("1");
  display.setCursor(0, 10);
  display.print("5");
  display.setCursor(10, 0);
  display.print("5");
  display.display();
}

void loop() {
}
