#ifndef _SMARTFLASH_H_
#define _SMARTFLASH_H_

#include <Arduino.h>

#define UPLINK_INTERVAL_OFFSET 0
#define CO2_MEASUREMENT_DELAY_OFFSET 1
#define CO2_ALTITUDE_OFFSET 2

class SmartFlash
{
public:
  SmartFlash();

  // Save and retrieve uplink interval
  bool saveUplinkIntervalIndex(uint8_t interval);
  uint8_t getUplinkIntervalIndex();

  // Index will be multiplied by 10000
  bool saveCO2MeasurementDelayIndex(uint8_t data);
  uint8_t getCO2MeasurementDelayIndex();

  // Index will be multiplied by 100
  bool saveCO2AltitudeIndex(uint8_t data);
  uint8_t getCO2AltitudeIndex();

private:
  uint8_t _intervalIndex = 0;
  uint8_t _co2MeasurementDelayIndex = 0;
  uint8_t _co2MeasurementDelayIndexOffset = 0;
  uint8_t _co2AltitudeIndex = 0;
  uint8_t _co2AltitudeIndexOffset = 0;
  bool writeToFlash(uint32_t offset, uint8_t *data, size_t length);
  bool readFromFlash(uint32_t offset, uint8_t *data, size_t length);
};

#endif // _SMARTFLASH_H_
#ifndef _SMARTFLASH_H_
#define _SMARTFLASH_H_

#include <Arduino.h>

#define UPLINK_INTERVAL_OFFSET 0
#define CO2_MEASUREMENT_DELAY_OFFSET 1
#define CO2_ALTITUDE_OFFSET 2

class SmartFlash
{
public:
  SmartFlash();

  // Save and retrieve uplink interval
  bool saveUplinkIntervalIndex(uint8_t interval);
  uint8_t getUplinkIntervalIndex();

  // Index will be multiplied by 10000
  bool saveCO2MeasurementDelayIndex(uint8_t data);
  uint8_t getCO2MeasurementDelayIndex();

  // Index will be multiplied by 100
  bool saveCO2AltitudeIndex(uint8_t data);
  uint8_t getCO2AltitudeIndex();

private:
  uint8_t _intervalIndex = 0;
  uint8_t _co2MeasurementDelayIndex = 0;
  uint8_t _co2MeasurementDelayIndexOffset = 0;
  uint8_t _co2AltitudeIndex = 0;
  uint8_t _co2AltitudeIndexOffset = 0;
  bool writeToFlash(uint32_t offset, uint8_t *data, size_t length);
  bool readFromFlash(uint32_t offset, uint8_t *data, size_t length);
};

#endif // _SMARTFLASH_H_
