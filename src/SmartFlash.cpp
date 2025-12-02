#include "SmartFlash.h"

SmartFlash::SmartFlash() {}

uint8_t SmartFlash::readUplinkIndex(uint8_t defaultValue)
{
  uint8_t data;
  // RUI3 api.system.flash.get returns true on success
  if (api.system.flash.get(FLASH_OFFSET_UPLINK_INDEX, &data, 1))
  {
    return data;
  }
  return defaultValue;
}

bool SmartFlash::saveUplinkIndex(uint8_t index)
{
  return api.system.flash.set(FLASH_OFFSET_UPLINK_INDEX, &index, 1);
}
