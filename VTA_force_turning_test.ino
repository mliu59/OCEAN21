
#include <HX711_ADC.h>
#include "HX711.h"
#define DOUT  3
#define CLK  2

HX711 scale;

float calibration_factor = 20000.21; //-7050 worked for my 440lb max scale setup

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  // Serial.println("Remove all weight from scale");
  //Serial.println("After readings begin, place known weight on scale");
  //Serial.println("Press + or a to increase calibration factor");
  // Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);
  
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  Serial.println("CLEARDATA"); //clear previous data in excel spreadsheet
  Serial.println("LABEL,Computer Time, load 2");

  long zero_factor = scale.read_average(); //Get a baseline reading
 // Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  //Serial.println(zero_factor);
}

void loop() {

  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  Serial.print("DATA,TIME");
  Serial.print(","); //indicate to move to next cell

  //Serial.print("Reading: ");
  Serial.print(scale.get_units(), 5);
  //Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
 // Serial.print(" calibration_factor: ");
  //Serial.print(calibration_factor);
  Serial.println();

  /*if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
  */
}
