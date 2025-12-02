#include "MagnetHandler.h"

MagnetHandler::MagnetHandler(uint8_t latchCutPin, uint8_t enableOverridePin, uint8_t latchResetPin)
    : _latchCutPin(latchCutPin),
      _enableOverridePin(enableOverridePin),
      _latchResetPin(latchResetPin)
{
}

void MagnetHandler::begin()
{
  pinMode(_latchCutPin, OUTPUT);       // LATCH_CUT
  pinMode(_enableOverridePin, OUTPUT); // EN_OR
  pinMode(_latchResetPin, OUTPUT);     // LATCH_RST

  digitalWrite(_latchResetPin, LOW); // SET LATCH RST TO LOW
  delay(200);
  digitalWrite(_enableOverridePin, HIGH); // Override
  delay(1000);
  digitalWrite(_latchCutPin, HIGH); // CUT POWER TO LATCH
}

void MagnetHandler::turnOff()
{
  digitalWrite(_latchResetPin, HIGH);    // Reset LATCH
  digitalWrite(_enableOverridePin, LOW); // Override
  delay(50);
  digitalWrite(_latchResetPin, LOW); // Real turn off the device
}
