#ifndef _SMARTFLASH_H_
#define _SMARTFLASH_H_

#include <Arduino.h>

// Define offsets for specific data
#define FLASH_OFFSET_UPLINK_INDEX 0

class SmartFlash
{
public:
  SmartFlash();

  // Returns the byte read from flash, or defaultValue if read fails
  uint8_t readUplinkIndex(uint8_t defaultValue);

  // Saves the byte to flash
  bool saveUplinkIndex(uint8_t index);
};

#endif
