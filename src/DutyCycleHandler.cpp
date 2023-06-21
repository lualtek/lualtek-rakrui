#include "DutyCycleHandler.h"

DutyCycleHandler::DutyCycleHandler(lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex)
    : defaultDutyCycleIndex(defaultDutyCycleIndex),
      uplinkInterval(dutyCycleCommandTable[defaultDutyCycleIndex]),
      previousMillis(0)
{
  // Setup duty cycle from flash memory if available or use default
  uint8_t uplinkIntervalFlash[1];
  api.system.flash.get(0, uplinkIntervalFlash, 1);
  changeDutyCycle(isDutyCycleIndex(uplinkIntervalFlash[0]) ? dutyCycleCommandTable[uplinkIntervalFlash[0]] : MINUTES_20_IN_MILLISECONDS);
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
  api.system.flash.set(0, commandIndexBuffer, 1);
}
