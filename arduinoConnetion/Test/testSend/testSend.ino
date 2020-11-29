#include <SoftwareSerial.h>

SoftwareSerial link(8, 9);

int a = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  link.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (a < 100) {
    link.println(a);
    a++;
    delay(500);
  }
}
