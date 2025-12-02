#include "DutyCycleHandler.h"

// Define table locally to avoid duplication in translation units
static const uint32_t DUTY_CYCLE_TABLE_MS[] = {
    3600000,  // 60 min
    2400000,  // 40 min
    1800000,  // 30 min
    1200000,  // 20 min
    900000,   // 15 min
    600000,   // 10 min
    300000,   // 5 min
    60000,    // 1 min
    43200000, // 12 hours
    86400000  // 24 hours
};

DutyCycleHandler::DutyCycleHandler(uint8_t defaultIndex)
    : _currentIndex(defaultIndex), _defaultIndex(defaultIndex)
{
  // Sanity check default
  if (!isValidIndex(_defaultIndex))
  {
    _defaultIndex = MINUTES_20; // Safe fallback
  }

  if (!isValidIndex(_currentIndex))
  {
    _currentIndex = _defaultIndex;
  }
}

bool DutyCycleHandler::isValidIndex(uint8_t index) const
{
  return index < (sizeof(DUTY_CYCLE_TABLE_MS) / sizeof(DUTY_CYCLE_TABLE_MS[0]));
}

uint32_t DutyCycleHandler::getIntervalMs(uint8_t index) const
{
  if (isValidIndex(index))
  {
    return DUTY_CYCLE_TABLE_MS[index];
  }
  return DUTY_CYCLE_TABLE_MS[_defaultIndex];
}

uint32_t DutyCycleHandler::getCurrentIntervalMs() const
{
  return getIntervalMs(_currentIndex);
}

uint8_t DutyCycleHandler::getCurrentIndex() const
{
  return _currentIndex;
}

bool DutyCycleHandler::setCycle(uint8_t index)
{
  if (isValidIndex(index))
  {
    _currentIndex = index;
    return true;
  }
  return false;
}
