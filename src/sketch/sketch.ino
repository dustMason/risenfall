#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define CONTINOUS_SERVOMIN 355 // this is the 'minimum' pulse length count (out of 4096)
#define CONTINOUS_SERVOMAX 455 // this is the 'maximum' pulse length count (out of 4096)
#define MICRO_SERVOMIN 500 // this is the 'minimum' pulse length count (out of 4096)
#define MICRO_SERVOMAX 2400 // this is the 'maximum' pulse length count (out of 4096)
#define CURVE -4.5 // log curve weight (between -10 and 10)

// the continuous servo on table 3:
// servo stops spinning when midi control is set at 84
// 84 / 128 = 0.6562
// 0.6562 * 700 = 459.34
// servo stops spinning at 405 pulsewidth
#define TABLE_3_SERVO_LIMIT_LEFT 22
#define TABLE_3_SERVO_LIMIT_RIGHT 23
#define TABLE_3_SERVO_OFF_PULSELENGTH 405

Adafruit_PWMServoDriver board1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver board2 = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver board3 = Adafruit_PWMServoDriver(0x42);
Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver(0x43);

boolean table_3_servo_limit_left = false;
boolean table_3_servo_limit_right = false;
boolean table_3_servo_turning_left = false;
boolean table_3_servo_turning_right = false;

void OnControlChange(byte channel, byte controlNumber, byte amount) {
  uint8_t pwmChannel = ((uint8_t) controlNumber) - 1;
  if (channel == 16) {
    uint16_t pulselen;
    if (pwmChannel < 8) {
      pulselen = map(amount, 0, 127, MICRO_SERVOMIN, MICRO_SERVOMAX);
    } else if (pwmChannel == 15) {
      if (
        (table_3_servo_limit_right && amount < 84) || // trying to turn right
        (table_3_servo_limit_left && amount > 85) // trying to turn left
      ) {
        pulselen = uint16_t(TABLE_3_SERVO_OFF_PULSELENGTH);
      } else {
        if (amount < 84) {
          table_3_servo_turning_left = false;
          table_3_servo_turning_right = true;
        } else if (amount > 85) {
          table_3_servo_turning_left = true;
          table_3_servo_turning_right = false;
        } else {
          table_3_servo_turning_left = false;
          table_3_servo_turning_right = false;
        }
        pulselen = map(amount, 0, 127, CONTINOUS_SERVOMIN, CONTINOUS_SERVOMAX);
      }
    } else {
      pulselen = map(amount, 0, 127, CONTINOUS_SERVOMIN, CONTINOUS_SERVOMAX);
    }
    servos.setPWM(pwmChannel, 0, pulselen);
  } else {
    uint16_t brightness;
    if (amount == 0) {
      brightness = 4096;
    } else {
      brightness = fscale(0, 127, 0, 4095, amount, CURVE);
    }
    if (channel == 1) {
      board1.setPWM(pwmChannel, 0, brightness);
    } else if (channel == 2) {
      board2.setPWM(pwmChannel, 0, brightness);
    } else if (channel == 3) {
      board3.setPWM(pwmChannel, 0, brightness);
    }
  }
}

void checkTable3ServoLimits() {
  table_3_servo_limit_left = (digitalRead(TABLE_3_SERVO_LIMIT_LEFT) == LOW);
  table_3_servo_limit_right = (digitalRead(TABLE_3_SERVO_LIMIT_RIGHT) == LOW);
  if (table_3_servo_limit_right && table_3_servo_turning_right) {
    servos.setPWM(15, 0, uint16_t(TABLE_3_SERVO_OFF_PULSELENGTH));
    table_3_servo_turning_right = false;
  } else if (table_3_servo_limit_left && table_3_servo_turning_left) {
    servos.setPWM(15, 0, uint16_t(TABLE_3_SERVO_OFF_PULSELENGTH));
    table_3_servo_turning_left = false;
  }
}

void setup() {
  board1.begin();
  board1.setPWMFreq(1600);
  board2.begin();
  board2.setPWMFreq(1600);
  board3.begin();
  board3.setPWMFreq(1600);
  servos.begin();
  servos.setPWMFreq(60); // Analog servos run at ~60 Hz updates
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
  pinMode(TABLE_3_SERVO_LIMIT_LEFT, INPUT_PULLUP);
  pinMode(TABLE_3_SERVO_LIMIT_RIGHT, INPUT_PULLUP);
}

void loop() {
  checkTable3ServoLimits();
  usbMIDI.read();
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  // zilch. strangely, controlChange doesn't work without a noteOn handler
}

// from http://playground.arduino.cc/Main/Fscale
float fscale(float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve) {
  float originalRange = 0;
  float newRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;
  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;
  curve = (curve * -.1); // postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function
  if (inputValue < originalMin) { inputValue = originalMin; }
  if (inputValue > originalMax) { inputValue = originalMax; }
  originalRange = originalMax - originalMin;
  if (newEnd > newBegin) {
    newRange = newEnd - newBegin;
  } else {
    newRange = newBegin - newEnd;
    invFlag = 1;
  }
  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / originalRange; // normalize to 0 - 1 float
  if (originalMin > originalMax ) { return 0; }
  if (invFlag == 0){
    rangedValue = (pow(normalizedCurVal, curve) * newRange) + newBegin;
  } else {
    rangedValue = newBegin - (pow(normalizedCurVal, curve) * newRange);
  }
  return rangedValue;
}
