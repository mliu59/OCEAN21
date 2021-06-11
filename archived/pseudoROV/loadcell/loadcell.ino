#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Servo.h>
#include "RTClib.h"
#include <HX711_ADC.h>
#include <HX711.h>
#define DOUT  3
#define CLK  2

#define minPWM 1100
#define maxPWM 1900

byte esc1pin=5;
byte esc2pin=6;

Servo ESC1;
Servo ESC2;

HX711 scale;

int m1 = 0;
int m2 = 0;

float reading = 0;

byte m1PWM = (A0); //M1 PWM values
byte m2PWM = (A1);

// change this to match your SD shield or module;
const int chipSelect = 53;

//File myFile;

float calibration_factor = 20000.21; //-7050 worked for my 440lb max scale setup
//unsigned long StartTime = millis(); //Is it okay to do this?

boolean calib = false;

void setup() {

  //Initialize Load Cell
  Wire.begin();

  scale.begin(DOUT, CLK);
  
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average();



  //Init Thrusters
  Serial.begin(9600);
  ESC1.attach(esc1pin);
  ESC2.attach(esc2pin);
  //stopThrust();

  //Get motor inputs from Pixhawk
  pinMode(m1PWM,INPUT);
  pinMode(m2PWM,INPUT);
  //Serial.print("Thrusters initalized");
}

void loop() {
  // put your main code here, to run repeatedly:
  //myFile = SD.open("test.csv", FILE_WRITE);


  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  reading = scale.get_units();

  Serial.println(reading);

  //motorInput(1100, 1100);
  
}
void stopThrust() {
  motorInput(minPWM, minPWM);
}

void datalog() {
  String dataString = String(millis()) + "," + String(m1) + "," + String(m2) + "," + String(reading);
  
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  dataFile.close();
}

void motorInput(int one, int two) {
  int r = clip(one, minPWM, maxPWM);
  int l = clip(two, minPWM, maxPWM);
  ESC1.writeMicroseconds(r);
  ESC2.writeMicroseconds(l);
  m1 = r;
  m2 = l;
  //radioWrite(String(r) +","+ String(l)+"," + String(m));
}

int clip(int val, int lower, int upper) {
  if (val < lower) {
    return lower;
  } else if (val > upper) {
    return upper;
  } else {
    return val;
  }
}
