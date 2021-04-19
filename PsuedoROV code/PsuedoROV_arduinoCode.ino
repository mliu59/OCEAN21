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
#define thrustLED 8
#define maxReading 20.0

#define PWMmax_R 1100
#define PWMmax_F 1900
#define stallPWM 1500
#define commonThrustPower 75
#define thrustTime 5000
#define loadCellFrequency 10 //in Hz

byte esc1pin=5;
byte esc2pin=6;

Servo ESC1;
Servo ESC2;

HX711 scale;


byte m1PWM = (A0); //M1 PWM values
byte m2PWM = (A1);
 
int pwm_value;

// change this to match your SD shield or module;
const int chipSelect = 10;

File myFile;

float calibration_factor = 20000.21; //-7050 worked for my 440lb max scale setup
unsigned long StartTime = millis(); //Is it okay to do this?

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);

  //Initialize Load Cell
   Wire.begin();

  scale.begin(DOUT, CLK);
  
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average();



  //Init Thrusters
  Serial.begin(57600);
  ESC1.attach(esc1pin);
  ESC2.attach(esc2pin);
  stopThrust();

  //Get motor inputs from Pixhawk
  pinMode(m1PWM,INPUT);
  pinMode(m2PWM,INPUT);
  //Serial.print("Thrusters initalized");

  //Initialize SD Card
  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

 
 


}

void loop() {
  // put your main code here, to run repeatedly:
   myFile = SD.open("test.txt", FILE_WRITE);


  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  float reading = scale.get_units();

  float motor1=pulseIn(m1PWM,HIGH); //should these be floats
  float motor2=pulseIn(m2PWM,HIGH);
  motorInput(motor1,motor2); //Get PWM values from pulse in 

 
 unsigned long CurrentTime = millis();
 unsigned long ElapsedTime = CurrentTime - StartTime;
 
 delay(1000); //allow for motors
 myFile.println("Time: " + String(ElapsedTime)+ "LoadCell Reading: " + String(reading)+ "Motor 1 PWM: " +String(motor1)+ "Motor 2 PWM: " +String(motor2)); 
 myFile.close();

}
void stopThrust() {
  motorInput(stallPWM, stallPWM);
}

void motorInput(int one, int two) {
  ESC1.writeMicroseconds(one);
  ESC2.writeMicroseconds(two);
}
