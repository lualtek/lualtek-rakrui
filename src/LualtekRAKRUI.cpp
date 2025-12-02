#include "LualtekRAKRUI.h"

LualtekRAKRUI::LualtekRAKRUI(
    const uint8_t appEui[8],
    const uint8_t appKey[16],
    uint8_t defaultDutyCycleIndex,
    PowerModeKind powerMode,
    Stream *debugStream)
    : _debugStream(debugStream),
      _dutyHandler(defaultDutyCycleIndex),
      _magnetHandler()
{
  memcpy(_appEui, appEui, 8);
  memcpy(_appKey, appKey, 16);
  _powerMode = powerMode;
}

// Private helper to handle logic, flash, and timers
void LualtekRAKRUI::processDutyCycleChange(uint8_t newIndex)
{
  if (_dutyHandler.setCycle(newIndex))
  {
    _flash.saveUplinkIndex(newIndex);

    // Restart timer with new interval
    api.system.timer.stop(RAK_TIMER_0);
    if (!api.system.timer.start(RAK_TIMER_0, _dutyHandler.getCurrentIntervalMs(), NULL))
    {
      _debugStream->println(F("Err: Timer restart failed"));
    }
    _debugStream->printf("Duty Cycle Updated: Index %d, %lu ms\r\n", newIndex, _dutyHandler.getCurrentIntervalMs());
  }
  else
  {
    _debugStream->println(F("Err: Invalid Duty Cycle Index"));
  }
}

void LualtekRAKRUI::onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload)
{
  if (!payload)
  {
    return;
  }

  if (payload->Port == PORT_CHANGE_INTERVAL)
  {
    if (payload->BufferSize > 0 && payload->Buffer != nullptr)
    {
      _debugStream->println(F("DL: Change Duty Cycle"));
      processDutyCycleChange(payload->Buffer[0]);
    }
  }
  else if (payload->Port == PORT_REJOIN)
  {
    _debugStream->println(F("DL: Rejoin Request. Rebooting..."));
    delay(100);
    api.system.reboot();
  }
  else if (payload->Port == PORT_TURN_OFF_MAGNET && _powerMode == POWER_MODE_MAGNET)
  {
    _debugStream->println(F("DL: Turn Off Magnet Request. Rebooting..."));
    _magnetHandler.turnOff();
  }
}

bool LualtekRAKRUI::setup()
{
  delay(1000);

  if (_powerMode == POWER_MODE_MAGNET)
  {
    _debugStream->println(F("Power Mode: Magnet"));
    _magnetHandler.begin();
  }

  _debugStream->println(F("--- Lualtek RAKRUI Init ---"));

  // 1. Restore Duty Cycle from Flash
  uint8_t savedIndex = _flash.readUplinkIndex(_dutyHandler.getCurrentIndex());
  // We update logic but don't save to flash (redundant) or start timers yet
  if (!_dutyHandler.setCycle(savedIndex))
  {
    _debugStream->println(F("Warning: Invalid saved index, using default"));
  }

  // 2. Hardware Setup
  if (!api.system.lpm.set(1))
  {
    _debugStream->println(F("Err: Low Power Mode set failed"));
    return false;
  }

  if (!api.lorawan.nwm.set())
  {
    _debugStream->println(F("Err: Network mode set failed"));
    return false;
  }

  if (!api.lorawan.appeui.set(_appEui, 8))
  {
    _debugStream->println(F("Err: AppEUI set failed"));
    return false;
  }

  if (!api.lorawan.appkey.set(_appKey, 16))
  {
    _debugStream->println(F("Err: AppKey set failed"));
    return false;
  }

  if (!api.lorawan.band.set(RAK_REGION_EU868))
  {
    _debugStream->println(F("Err: Band set failed"));
    return false;
  }

  if (!setClass(RAK_LORA_CLASS_A))
  {
    _debugStream->println(F("Err: Set Class failed"));
    return false;
  }

  if (!api.lorawan.njm.set(RAK_LORA_OTAA))
  {
    _debugStream->println(F("Err: Join Mode set failed"));
    return false;
  }

  return true;
}

bool LualtekRAKRUI::join(uint32_t attemptTimeoutMs, JoinBehavior behavior)
{
  // SF12 (DR 0) is very slow and consumes high airtime.
  // Consider allowing ADR to handle this, or start at DR_0 and move up.
  api.lorawan.dr.set(0);

  _debugStream->println(F("LoRaWAN Joining..."));

  if (!api.lorawan.join())
  {
    _debugStream->println(F("Err: Join command failed"));
    return false;
  }

  unsigned long startAttempt = millis();

  // Non-blocking wait (with timeout)
  while (api.lorawan.njs.get() == 0)
  {
    if (millis() - startAttempt > attemptTimeoutMs)
    {
      if (behavior == JOIN_ONCE)
      {
        _debugStream->println(F("Err: Join Timeout. Giving up."));
        return false;
      }

      _debugStream->println(F("Err: Join Timeout. Going to sleep for 2 minutes then try again."));
      api.system.sleep.all(120000); // Sleep for 2 minutes
      startAttempt = millis();
    }

    api.lorawan.join();
    // Yield to system background tasks
    delay(5000);
  }

  // Post-Join Configuration
  if (!api.lorawan.adr.set(true))
  {
    _debugStream->println(F("Warning: ADR enable failed"));
  }

  if (!api.lorawan.rety.set(1))
  {
    _debugStream->println(F("Warning: Set retry failed"));
  }

  if (!api.lorawan.cfm.set(false))
  {
    _debugStream->println(F("Warning: Set confirm failed"));
  }

  // Debug Info
  uint8_t assigned_addr[4];
  api.lorawan.daddr.get(assigned_addr, 4);

  _debugStream->printf("Joined! DevAddr: %02X%02X%02X%02X\r\n",
                       assigned_addr[0], assigned_addr[1], assigned_addr[2], assigned_addr[3]);
  _debugStream->printf("Uplink Interval: %lu ms\r\n", _dutyHandler.getCurrentIntervalMs());

  return true;
}

bool LualtekRAKRUI::setClass(RAK_LORA_CLASS classType)
{
  return api.lorawan.deviceClass.set(classType);
}

bool LualtekRAKRUI::setupTimers(void (*callback)())
{
  if (!api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)callback, RAK_TIMER_PERIODIC))
  {
    return false;
  }
  // Start with the interval calculated by the handler
  if (!api.system.timer.start(RAK_TIMER_0, _dutyHandler.getCurrentIntervalMs(), NULL))
  {
    return false;
  }
  return true;
}

bool LualtekRAKRUI::send(uint8_t size, uint8_t *data, uint8_t fPort)
{
  if (api.lorawan.send(size, data, fPort, false))
  {
    // _debugStream->println(F("Packet Queued"));
    return true;
  }
  _debugStream->println(F("Err: Send Failed"));
  return false;
}

uint32_t LualtekRAKRUI::getUplinkIntervalMs()
{
  return _dutyHandler.getCurrentIntervalMs();
}
