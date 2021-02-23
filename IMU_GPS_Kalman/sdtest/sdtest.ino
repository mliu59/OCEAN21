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
#include <EEPROM.h>
#include <EEWrap.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <BasicLinearAlgebra.h>

#include <TinyGPS.h>
#include <SoftwareSerial.h>
SoftwareSerial ss(8, 9);

using namespace BLA;

#define calib true
#define IMU_READ_PERIOD_MS 25
#define GPS_READ_PERIOD_MS 1000

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define chipSelect 4
MPU9250 mpu;
TinyGPS gps;

//unsigned long t0 = 0;
//unsigned long t1;
unsigned long newGPSTime;
float deltaT = 1.0; //in seconds

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
  
  B.Fill(0);
  u.Fill(0);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Wire.begin();
  ss.begin(9600);
  while (!Serial) {}

  
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

  
  if (!mpu.setup(0x68)) {
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
  mpu.setMagneticDeclination(-3.11);

  Serial.print("Roll");
  Serial.print(",");
  Serial.print("Pitch");
  Serial.print(",");
  Serial.print("Yaw");
  Serial.print(",");
  Serial.println("MagX");

  while (!ss.available()) {}
}

void loop() {

  //String dataString;

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  
  //File dataFile = SD.open("datalog.csv", FILE_WRITE);
  //boolean newData = false;
  
  /*
  for (unsigned long start = millis(); millis() - start < GPS_READ_PERIOD_MS;) {
    while (ss.available()) {
      char c = ss.read(); //while ss is transmitting data, read each character
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true; //if the sentence is valid, flip the newData variable so that the
    }*/
    if (mpu.update()) {
      
      //mpu.printRollPitchYaw();
      Serial.print(mpu.getRoll(), 2);
      Serial.print(",");
      Serial.print(mpu.getPitch(), 2);
      Serial.print(",");
      Serial.print(mpu.getYaw(), 2);
      Serial.print(",");
      Serial.println(mpu.getMagX(), 2);
  
      display.setCursor(0, 0);
      display.clearDisplay();
      display.println("Roll  X " + String(mpu.getRoll(), 2));
      display.println("Pitch Y " + String(mpu.getPitch(), 2));
      display.println("Yaw   Z " + String(mpu.getYaw(), 2));
      display.display();
      
      delay(IMU_READ_PERIOD_MS);
    }
    
  /*}
  if (newData) {
    gps.f_get_position(&lat, &lon, &age); //read from gps encoded data and store them.
    newGPSTime = millis();
    P0 = P;
    x0 = x;
    
    z = {lat, lon, aN, aE};
    
    //float NS = TinyGPS::distance_between(slat, slon, flat, slon);
    A = {1, 0, deltaT, 0, deltaT*deltaT/2, 0, 
         0, 1, 0, deltaT, 0, deltaT*deltaT/2,
         0, 0, 1, 0, deltaT, 0,
         0, 0, 0, 1, 0, deltaT,
         0, 0, 0, 0, 1, 0,
         0, 0, 0, 0, 0, 1};
    x_ = (A * x0) + (B * u);
    P_ = (A * P0 * ~A) + Q;

    Matrix<4, 4> temp = H * P_ * ~H + R;
    K = P_ * ~H * temp.Inverse();
    x = x_ + K * (z - (H * x_));
    P = (I - (K * H)) * P_;

    dataString = String(millis()) + "," + String(lat, 6) + "," + String(lon, 6);
    
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      //Serial.println(dataString);
    }
    else {
      Serial.println("error opening datalog.csv");
    }
  }
  */
}
