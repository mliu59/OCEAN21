#include <HX711_ADC.h>
#include "HX711.h"
#define DOUT1  3
#define CLK1  2
#define DOUT2 5
#define CLK2 4
//#define M_PI 3.141592653

HX711 scale1;
HX711 scale2;

float calibration_factor1 = 20000.21;
float calibration_factor2 = 20000.21;//-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
  scale1.begin(DOUT1, CLK1);
  scale2.begin(DOUT2, CLK2);
  
  scale1.set_scale();
  scale2.set_scale();
  scale1.tare();
  scale2.tare();//Reset the scale to 0

  long zero_factor1 = scale1.read_average(); //Get a baseline reading
  long zero_factor2 = scale2.read_average();
  Serial.print("Zero factor 1: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor1);
  Serial.print("Zero factor 2: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor2);
}

void loop() {

  scale1.set_scale(calibration_factor1);
  scale1.set_scale(calibration_factor2);//Adjust to this calibration factor

  float reading1 = scale1.get_units();
  float reading2 = scale2.get_units();
  
  Serial.print("Scale 1 Magnitude: ");
  Serial.println(reading1, 5);
  Serial.print("Scale 2 Magnitude: ");
  Serial.println(reading2, 5);

  int resolvedAngle = (int)(atan2(reading1, reading2)/M_PI*360);
  Serial.println();
  Serial.println("Angle: " + String(resolvedAngle));
  Serial.println();

  delay(200);
}
