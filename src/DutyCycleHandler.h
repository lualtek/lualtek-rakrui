#ifndef _DUTYCYCLEHANDLER_H_
#define _DUTYCYCLEHANDLER_H_

#include <Arduino.h>

// Using standard types for clarity
enum DutyCycleIndex : uint8_t
{
  MINUTES_60 = 0,
  MINUTES_40 = 1,
  MINUTES_30 = 2,
  MINUTES_20 = 3,
  MINUTES_15 = 4,
  MINUTES_10 = 5,
  MINUTES_5 = 6,
  MINUTES_1 = 7,
  MINUTES_720 = 8,
  MINUTES_1440 = 9
};

class DutyCycleHandler
{
public:
  DutyCycleHandler(uint8_t defaultIndex);

  // Returns true if index is valid
  bool isValidIndex(uint8_t index) const;

  // Returns milliseconds for a specific index
  uint32_t getIntervalMs(uint8_t index) const;

  // Returns the current milliseconds
  uint32_t getCurrentIntervalMs() const;

  // Returns the current index
  uint8_t getCurrentIndex() const;

  // Sets the index, returns true if changed and valid
  bool setCycle(uint8_t index);

private:
  uint8_t _currentIndex;
  uint8_t _defaultIndex;
};

#endif
