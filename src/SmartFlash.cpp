#include "SmartFlash.h"

SmartFlash::SmartFlash() {}

bool SmartFlash::begin()
{
  getUplinkIntervalIndex();
  getCO2MeasurementDelayIndex();
  getCO2AltitudeIndex();
  delay(500);
}

bool SmartFlash::saveUplinkIntervalIndex(uint8_t interval)
{
  _intervalIndex = interval;
  return writeToFlash(UPLINK_INTERVAL_OFFSET, &interval, sizeof(_intervalIndex));
}

uint8_t SmartFlash::getUplinkIntervalIndex()
{
  readFromFlash(UPLINK_INTERVAL_OFFSET, &_intervalIndex, sizeof(_intervalIndex));
  return _intervalIndex;
}

bool SmartFlash::saveCO2MeasurementDelayIndex(uint8_t data)
{
  _co2MeasurementDelayIndex = data;
  _co2MeasurementDelayIndexOffset = UPLINK_INTERVAL_OFFSET + sizeof(_intervalIndex);
  return writeToFlash(_co2MeasurementDelayIndexOffset, &data, sizeof(_co2MeasurementDelayIndex));
}

uint8_t SmartFlash::getCO2MeasurementDelayIndex()
{
  readFromFlash(_co2MeasurementDelayIndexOffset, &_co2MeasurementDelayIndex, sizeof(_co2MeasurementDelayIndex));
  return _co2MeasurementDelayIndex;
}

bool SmartFlash::saveCO2AltitudeIndex(uint8_t data)
{
  _co2AltitudeIndex = data;
  _co2AltitudeIndexOffset = _co2MeasurementDelayIndexOffset + sizeof(_co2MeasurementDelayIndex);
  return writeToFlash(_co2AltitudeIndexOffset, &data, sizeof(_co2AltitudeIndex));
}

uint8_t SmartFlash::getCO2AltitudeIndex()
{
  readFromFlash(_co2AltitudeIndexOffset, &_co2AltitudeIndex, sizeof(_co2AltitudeIndex));
  return _co2AltitudeIndex;
}

bool SmartFlash::writeToFlash(uint32_t offset, uint8_t *data, size_t length)
{
  return api.system.flash.set(offset, data, length);
}

bool SmartFlash::readFromFlash(uint32_t offset, uint8_t *data, size_t length)
{
  return api.system.flash.get(offset, data, length);
}
