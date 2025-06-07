#include "Arduino.h"
#include "CAT5171.h"
#include <Wire.h>

CAT5171::CAT5171()
{
  _address = ADDRESS0;

   Wire.begin();
}

void CAT5171::setWiper(byte position)
{
  Wire.beginTransmission(_address);
  
  Wire.write(B00000000);
  Wire.write(position);
  Wire.endTransmission();
}

byte CAT5171::getWiper(byte deviceIndex) {
    if (deviceIndex > 1) return 0;

    byte pos = 0;
    Wire.requestFrom(_addresses[deviceIndex], (uint8_t)1);
    while (Wire.available()) {
        pos = Wire.read();
    }
    return pos;
}

void CAT5171::shutDown()
{  
  Wire.beginTransmission(_address);
  
  Wire.write(B00100000);
  Wire.write((byte)0);
  Wire.endTransmission();
}

void CAT5171::switchToSecondDevice()
{
  _address = ADDRESS1;
}

void CAT5171::setWiperBoth(byte position) {
    for (int i = 0; i < 2; ++i) {
        Wire.beginTransmission(_addresses[i]);
        Wire.write(B00000000); // Command byte for setting wiper
        Wire.write(position);
        Wire.endTransmission();
    }
}