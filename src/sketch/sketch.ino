#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void OnControlChange(byte channel, byte controlNumber, byte amount) {
  uint16_t brightness;
  if (amount == 0) {
    brightness = 4096;
  } else {
    brightness = amount * 32;
  }
  pwm.setPWM(((uint8_t) controlNumber) - 1, 0, brightness);
}

void setup() {
  pwm.begin();
  pwm.setPWMFreq(1600);
  // strangely, controlChange doesn't work without a noteOn handler
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange) ;
}

void loop() {
  usbMIDI.read();
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  // do nothing!
}