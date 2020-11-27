#include <Servo.h>
#include <HX711_ADC.h>
#include "HX711.h"
#define DOUT  3
#define CLK  2
#define PWMmax_R 1100
#define PWMmax_F 1900
#define PWMstall 1500
#define commonThrustPower 75
#define thrustTime 5000
#define loadCellFrequency 5 //in Hz
#define thresholdForceChangePerSecond 2.0 //if the rate of change in force deceted by the load cell is more than this value, motors will engage 

byte esc1pin = 5;
byte esc2pin = 6;
Servo ESC1;
Servo ESC2;

const int forwardPWM = PWMstall + ((PWMmax_F - PWMstall) * commonThrustPower / 100);

HX711 scale;

float calibration_factor = 20000.21; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
  ESC1.attach(esc1pin);
  ESC2.attach(esc2pin);
  stopThrust();
  delay(5000);

  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
}

void loop() {
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  float initReading = scale.get_units();
  delay(1000 / loadCellFrequency); //5 Hz readings
  if (abs(initReading - scale.get_units) > ((double)thresholdForceChangePerSecond / loadCellFrequency)) {
    forwardThrust();
    delay(thrustTime);
    stopThrust();
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
