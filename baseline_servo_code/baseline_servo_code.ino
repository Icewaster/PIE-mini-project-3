#include <Servo.h>

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

void setup() {
  servo1.attach(10);
  servo2.attach(11);
  servo3.attach(12);
  servo4.attach(13);

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  delay(2000);
  servo1.write(0);
  servo2.write(180);
  servo3.write(0);
  servo4.write(180);
  delay(2000);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
}

void loop() {
  // for (int angle = 90; angle <= 180; angle++) {
  //   my_servo.write(angle);
  //   my_servo2.write(180 - angle);

  // }

  // delay(1000);

  // for (int angle = 180; angle >= 90; angle--) {
  //   my_servo.write(angle);
  //   my_servo2.write(180 - angle);

  // }

  // delay(1000);
}
