#include "DutyCycleHandler.h"

DutyCycleHandler::DutyCycleHandler(lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex)
    : defaultDutyCycleIndex(defaultDutyCycleIndex),
      uplinkInterval(dutyCycleCommandTable[defaultDutyCycleIndex]),
      previousMillis(0)
{
  // Setup duty cycle from flash memory if available or use default
  uint8_t uplinkIntervalFlashIndex = smartflash.getUplinkIntervalIndex();
  changeDutyCycle(isDutyCycleIndex(uplinkIntervalFlashIndex) ? dutyCycleCommandTable[uplinkIntervalFlashIndex] : MINUTES_20_IN_MILLISECONDS);
}

bool DutyCycleHandler::isDutyCycleIndex(unsigned int commandIndex)
{
  return commandIndex >= 0 && commandIndex <= sizeof(dutyCycleCommandTable) - 1;
}

void DutyCycleHandler::changeDutyCycle(int commandIndex)
{
  if (!isDutyCycleIndex(commandIndex))
  {
    return;
  }

  uplinkInterval = dutyCycleCommandTable[commandIndex];
  uint8_t commandIndexBuffer[1] = {commandIndex};
  smartflash.saveUplinkIntervalIndex(commandIndex);
}
