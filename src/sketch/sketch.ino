#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
bool lightOn = false;

void OnControlChange(byte channel, byte controlNumber, byte amount) {
  uint16_t brightness;
  if (amount == 0) {
    brightness = 4096;
  } else {
    brightness = amount * 32;
  }
  pwm.setPWM(((uint8_t) controlNumber) - 1, 0, brightness);
  if (lightOn) {
    digitalWrite(13, LOW);
    delay(100);
  } else {
    digitalWrite(13, HIGH);
    delay(100);
  }
}

void setup() {
  pwm.begin();
  pwm.setPWMFreq(1600);
  pinMode(13, OUTPUT);
  // strangely, controlChange doesn't work without a noteOn handler
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange) ;
  digitalWrite(13, HIGH);
}

void loop() {
  usbMIDI.read();
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  // do nothing!
}
