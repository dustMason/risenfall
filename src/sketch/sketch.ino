#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVOMIN  100 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  700 // this is the 'maximum' pulse length count (out of 4096)

Adafruit_PWMServoDriver table1 = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver(0x41);

void OnControlChange(byte channel, byte controlNumber, byte amount) {
  if (channel == 2) {
    uint16_t pulselen = map(amount, 0, 127, SERVOMIN, SERVOMAX);
    servos.setPWM(((uint8_t) controlNumber), 0, pulselen);
  } else {
    uint16_t brightness;
    if (amount == 0) {
      brightness = 4096;
    } else {
      brightness = amount * 32;
    }
    table1.setPWM(((uint8_t) controlNumber) - 1, 0, brightness);
  }
}

void setup() {
  table1.begin();
  table1.setPWMFreq(1600);
  servos.begin();
  servos.setPWMFreq(60); // Analog servos run at ~60 Hz updates
  // strangely, controlChange doesn't work without a noteOn handler
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange) ;
}

void loop() {
  usbMIDI.read();
}

void OnNoteOn(byte channel, byte note, byte velocity) {
  // zilch
}
