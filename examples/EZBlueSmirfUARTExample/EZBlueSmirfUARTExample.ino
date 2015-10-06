// Use hardware or software serial. In this instance we use Seria1, which is pin 0 & 1 on
// teensy LC. DO NOT SET AS SERIAL or bad things will happen.
#define Bluetooth Serial1
//#define BT_NOVERBOSE

// Including this file will pull in Bluetooth.
#include "EZBlueSmirfUART.h"

void setup() {
  Serial.begin(9600);
  BT_BondPrivatelyWith("00066673E605", "00066673E631");
}

void loop() {
  BT_Echo();
}