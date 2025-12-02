#ifndef MAGNET_HANDLER_H
#define MAGNET_HANDLER_H

#include <Arduino.h>

// Based on our Lualtek I/O v2 board design
// PA10 - LATCH_CUT
// PA1  - EN_OR
// PA9  - LATCH_RST
#define LATCH_CUT_PIN 10
#define ENABLE_OVERRIDE_PIN 1
#define LATCH_RESET_PIN 9

class MagnetHandler
{
public:
  MagnetHandler(
      uint8_t latchCutPin = LATCH_CUT_PIN,
      uint8_t enableOverridePin = ENABLE_OVERRIDE_PIN,
      uint8_t latchResetPin = LATCH_RESET_PIN);

  void begin();
  void turnOff();

private:
  uint8_t _latchCutPin;
  uint8_t _enableOverridePin;
  uint8_t _latchResetPin;
};

#endif
