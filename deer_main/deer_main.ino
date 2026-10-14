#include <Servo.h>
#include <Adafruit_MotorShield.h>
#include <vector>
#include <algorithm>

// Motor Shield and head motor
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(4);

// Front-left leg servo
Servo my_servo;
const int SERVO_PIN = 13;

// Ultrasonic sensor
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;

// Settings
const int DETECT_DISTANCE = 90;
const int RESET_DISTANCE = 100;
const int CLOSE_DISTANCE = 20;
const int MIN_ANGLE = 90;
const int MAX_ANGLE = 180;
const int MOTOR_TIME = 500;
const int NEAR_READINGS_REQUIRED = 3;
const int FAR_READINGS_REQUIRED = 6;
const unsigned long NO_ECHO_TIMEOUT = 3000;

bool head_is_up = false;
bool person_detected = false;
bool has_filtered_distance = false;

int current_angle = MIN_ANGLE;
int target_angle = MIN_ANGLE;
int near_count = 0;
int far_count = 0;
float filtered_distance = 0;
unsigned long last_valid_reading = 0;

void setup() {
  Serial.begin(115200);

  if (!AFMS.begin()) {
    Serial.println("Motor Shield not found!");
    while (1);
  }

  my_servo.attach(SERVO_PIN);
  my_servo.write(MIN_ANGLE);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);
  myMotor->run(RELEASE);
  last_valid_reading = millis();
  delay(1000);
  Serial.println("Deer ready!");
}

void loop() {
  float distance = read_filtered_distance();
  if (distance >= 0) {
    last_valid_reading = millis();
    filtered_distance = smoothenDistance(distance);

    Serial.print("Distance: ");
    Serial.print(filtered_distance);
    Serial.println(" cm");

    detectPerson();
    calculateLegAngle();

  } else {
    // Readings that are either failed or there is no object/person nearby
    near_count = 0;
    far_count = 0;
    Serial.println("No valid echo");
  }
  handleHead();
  setLeg();

  delay(20);
}

float read_filtered_distance() {
  std::vector<float> readings;
  int count = 0;

  for (int i = 0; i < 5; i++) {
    long duration = pulseDuration();
    if (duration > 0) {
      float cm = duration * 0.0343 / 2;
      if (cm >= 2 && cm <= 400) {
        readings.push_back(cm);
      }
    }
    delay(5);
  }

  if (readings.size() <= 3) {
    return -1;
  }
  return getMedian(readings);
}

// Send a pulse to the trigger input of the ultrasonic sensor and record echo duration
long pulseDuration() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  return pulseIn(ECHO_PIN, HIGH, 12000);
}

// Get the median of a vector
float getMedian(std::vector<float> v) {
  if (v.empty()) {
    return 0.0; // Handle empty vector case
  }

  size_t n = v.size();
  size_t mid = n / 2;

  if (n % 2 != 0) {
    std::nth_element(v.begin(), v.begin() + mid, v.end());
    return static_cast<double>(v[mid]);
  } else {
    std::nth_element(v.begin(), v.begin() + mid, v.end());
    int high_mid = v[mid];
    
    std::nth_element(v.begin(), v.begin() + mid - 1, v.end());
    int low_mid = v[mid - 1];
    
    return (static_cast<float>(low_mid) + high_mid) / 2.0;
  }
}

// "Smoothen" the distance reading via a rolling exponential average
float smoothenDistance(float distance) {
  if (!has_filtered_distance) {
    filtered_distance = distance;
    has_filtered_distance = true;
  } else {
    filtered_distance =
    0.7 * filtered_distance + 0.3 * distance;
  }

  return filtered_distance;
}

void detectPerson() {
  if (filtered_distance < DETECT_DISTANCE) {
    near_count++;
    far_count = 0;
    if (near_count >= NEAR_READINGS_REQUIRED) {
      person_detected = true;
    }
  }
  // Confirm the person has left
  else if (filtered_distance > RESET_DISTANCE) {
    far_count++;
    near_count = 0;
    if (far_count >= FAR_READINGS_REQUIRED) {
      person_detected = false;
    }
  } else {
    near_count = 0;
    far_count = 0;
  }
}

// Calculate leg angle based on filtered distance
void calculateLegAngle() {
  if (person_detected) {
    if (filtered_distance <= CLOSE_DISTANCE) {
      target_angle = MAX_ANGLE;
    } else if (filtered_distance >= DETECT_DISTANCE) {
      target_angle = MIN_ANGLE;
    } else {
      target_angle = MIN_ANGLE + (int)((DETECT_DISTANCE - filtered_distance) * (MAX_ANGLE - MIN_ANGLE) / (DETECT_DISTANCE - CLOSE_DISTANCE));
    }
  } else {
  // Return leg to 90 degrees when person leaves
    target_angle = MIN_ANGLE;
  }
}

void handleHead() {
  // Lower head if readings have been missing for too long
  if (person_detected &&
  millis() - last_valid_reading >= NO_ECHO_TIMEOUT) {
    person_detected = false;
    target_angle = MIN_ANGLE;
  }
  // Move head only when its state changes
  if (person_detected && !head_is_up) {
    motorUp();
    head_is_up = true;
    Serial.println("Head raised");
  }
  if (!person_detected && head_is_up) {
    motorDown();
    head_is_up = false;
    Serial.println("Head lowered");
  }
}

// Raise head
void motorUp() {
  myMotor->setSpeed(150);
  myMotor->run(BACKWARD);
  delay(MOTOR_TIME);
  myMotor->run(RELEASE);
}

// Lower head
void motorDown() {
  myMotor->setSpeed(150);
  myMotor->run(FORWARD);
  delay(MOTOR_TIME);
  myMotor->run(RELEASE);
}

// Move the leg servo to target angle
void setLeg() {
  // Move leg smoothly toward target angle
  if (current_angle < target_angle) {
    current_angle += 2;
  if (current_angle > target_angle) {
    current_angle = target_angle;
  }
  my_servo.write(current_angle);
  } else if (current_angle > target_angle) {
    current_angle -= 2;
    if (current_angle < target_angle) {
      current_angle = target_angle;
    }
    my_servo.write(current_angle);
  }
}