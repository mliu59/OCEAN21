#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Servo.h>
#include "RTClib.h"
#include <HX711_ADC.h>
#include <HX711.h>
#define DOUT  3
#define CLK  2

#define PWMmax_R 1100
#define PWMmax_F 1900
#define stallPWM 1500

byte esc1pin=5;
byte esc2pin=6;

Servo ESC1;
Servo ESC2;

HX711 scale;


byte m1PWM = (A0); //M1 PWM values
byte m2PWM = (A1);

// change this to match your SD shield or module;
const int chipSelect = 10;

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

  /*
  //Initialize SD Card
  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  */

  if (calib) {
    motorInput(1900, 1900);
    delay(15000);
  
    motorInput(1100, 1100);
    delay(15000);
  
    motorInput(1500, 1500);
    delay(2000);
    motorInput(1900, 1100);
    delay(2000);
  } else {

    motorInput(1100, 1100);
    delay(15000);
  
    motorInput(1900, 1900);
    delay(6000);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  //myFile = SD.open("test.csv", FILE_WRITE);


  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  float reading = scale.get_units();

  Serial.println(reading);

  motorInput(1100, 1100);

  //int motor1=pulseIn(m1PWM,HIGH);
  //int motor2=pulseIn(m2PWM,HIGH);
  //motorInput(motor1,motor2); //Get PWM values from pulse in 

  //unsigned long ElapsedTime = millis() - StartTime;
 
  //delay(1000); //allow for motors
  //myFile.println(String(ElapsedTime)+ "," + String(reading)+ "," +String(motor1)+ "," +String(motor2)); 
  //myFile.close();
  delay(50);

}
void stopThrust() {
  motorInput(stallPWM, stallPWM);
}

void motorInput(int one, int two) {
  Serial.println(String(one) + " , " + String(two));
  ESC1.writeMicroseconds(one);
  ESC2.writeMicroseconds(two);
}
