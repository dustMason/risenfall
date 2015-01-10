#include <math.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN  100 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  700 // this is the 'maximum' pulse length count (out of 4096)
#define CURVE -4.5 // log curve weight (between -10 and 10)

Adafruit_PWMServoDriver table1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver table2 = Adafruit_PWMServoDriver(0x42);
Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver(0x41);

void OnControlChange(byte channel, byte controlNumber, byte amount) {
  if (channel == 16) {
    uint16_t pulselen = map(amount, 0, 127, SERVOMIN, SERVOMAX);
    servos.setPWM(((uint8_t) controlNumber), 0, pulselen);
  } else {
    uint16_t brightness;
    uint8_t pwmChannel = ((uint8_t) controlNumber) - 1;
    if (amount == 0) {
      brightness = 4096;
    } else {
      // brightness = amount * 32;
      brightness = fscale(0, 127, 0, 4095, amount, CURVE);
    }
    if (channel == 1) {
      table1.setPWM(pwmChannel, 0, brightness);
    } else if (channel == 2) {
      table2.setPWM(pwmChannel, 0, brightness);
    }
  }
}

void setup() {
  table1.begin();
  table1.setPWMFreq(1600);
  servos.begin();
  servos.setPWMFreq(60); // Analog servos run at ~60 Hz updates
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange) ;
}

void loop() { usbMIDI.read(); }

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
