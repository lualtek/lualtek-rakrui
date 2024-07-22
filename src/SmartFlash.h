#ifndef _SMARTFLASH_H_
#define _SMARTFLASH_H_

#include <Arduino.h>

#define UPLINK_INTERVAL_OFFSET 0

class SmartFlash
{
public:
  SmartFlash();

  // Save and retrieve uplink interval
  bool saveUplinkIntervalIndex(uint8_t interval);
  uint8_t getUplinkIntervalIndex();

  // Save and retrieve CO2 sensor data
  bool saveCO2MeasurementDelay(uint8_t data);
  uint8_t getCO2MeasurementDelay();

  bool saveCO2Altitude(uint8_t data);
  uint8_t getCO2Altitude();

private:
  uint8_t _intervalIndex = 0;
  uint8_t _co2MeasurementDelay = 0;
  uint8_t _co2MeasurementDelayOffset = 0;
  uint8_t _co2Altitude = 0;
  uint8_t _co2AltitudeOffset = 0;
  bool writeToFlash(uint32_t offset, uint8_t *data, size_t length);
  bool readFromFlash(uint32_t offset, uint8_t *data, size_t length);
};

#endif // SMARTFLASH_H
