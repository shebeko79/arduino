/* Sweep
  by BARRAGAN <http://barraganstudio.com>
  This example code is in the public domain.

  modified 28 May 2015
  by Michael C. Miller
  modified 8 Nov 2013
  by Scott Fitzgerald

  http://arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards


void setup() {
  Serial.begin(9600);
  myservo.attach(D0);  // attaches the servo on GIO2 to the servo object
  myservo.write(90);
}

void setServo(int val)
{
  Serial.println(val);
  myservo.write(val);
  delay(2000);
}

void loop() {
  int pos;

  setServo(75);
  setServo(90);
  setServo(110);
  setServo(90);
}
