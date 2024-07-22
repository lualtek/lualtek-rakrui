#include "SmartFlash.h"

SmartFlash::SmartFlash() {}

bool SmartFlash::saveUplinkIntervalIndex(uint8_t interval)
{
  _intervalIndex = interval;
  return writeToFlash(UPLINK_INTERVAL_OFFSET, &interval, sizeof(_intervalIndex));
}

uint8_t SmartFlash::getUplinkIntervalIndex()
{
  return readFromFlash(UPLINK_INTERVAL_OFFSET, &_intervalIndex, sizeof(_intervalIndex));
}

bool SmartFlash::saveCO2MeasurementDelay(uint8_t data)
{
  _co2MeasurementDelay = data;
  _co2MeasurementDelayOffset = UPLINK_INTERVAL_OFFSET + sizeof(_intervalIndex);
  return writeToFlash(_co2MeasurementDelayOffset, &data, sizeof(_co2MeasurementDelay));
}

uint8_t SmartFlash::getCO2MeasurementDelay()
{
  return readFromFlash(_co2MeasurementDelayOffset, &_co2MeasurementDelay, sizeof(_co2MeasurementDelay));
}

bool SmartFlash::saveCO2Altitude(uint8_t data)
{
  _co2Altitude = data;
  _co2AltitudeOffset = _co2MeasurementDelayOffset + sizeof(_co2MeasurementDelay);
  return writeToFlash(_co2AltitudeOffset, &data, sizeof(_co2Altitude));
}

bool SmartFlash::writeToFlash(uint32_t offset, uint8_t *data, size_t length)
{
  return api.system.flash.set(offset, data, length);
}

bool SmartFlash::readFromFlash(uint32_t offset, uint8_t *data, size_t length)
{
  return api.system.flash.get(offset, data, length);
}
