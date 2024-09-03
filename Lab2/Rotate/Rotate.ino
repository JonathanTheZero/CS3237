#include <ESP32Servo.h>
#define SERVO_PIN 18

Servo myservo;
int pos;

void setup() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 1000, 2000);

  //start in the middle
  pos = 90;
  myservo.write(pos);
}

void loop() {
  for (pos = 90; pos <= 180; pos += 1) {
    myservo.write(pos);
    delay(10);
  }

  for (pos = 180; pos >= 0; pos -= 1) {
    myservo.write(pos);
    delay(10);
  }

  for (pos = 0; pos <= 90; pos += 1) {
    myservo.write(pos);
    delay(10);
  }

  delay(2000);
}
