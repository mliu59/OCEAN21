#include <SoftwareSerial.h>

SoftwareSerial link(8, 9);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  link.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (link.available()) {
    Serial.write(link.read());
  }
  while (Serial.available()) {
    link.write(Serial.read());
  }
}
