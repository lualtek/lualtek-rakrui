#include "LualtekRAKRUI.h"

LualtekRAKRUI::LualtekRAKRUI(
    const uint8_t appEui[8],
    const uint8_t appKey[16],
    uint8_t defaultDutyCycleIndex,
    Stream *debugStream)
    : _debugStream(debugStream),
      _dutyHandler(defaultDutyCycleIndex),
      _hasCustomDevEui(false)
{
  memcpy(_appEui, appEui, 8);
  memcpy(_appKey, appKey, 16);
}

LualtekRAKRUI::LualtekRAKRUI(
    const uint8_t devEui[8],
    const uint8_t appEui[8],
    const uint8_t appKey[16],
    uint8_t defaultDutyCycleIndex,
    Stream *debugStream)
    : _debugStream(debugStream),
      _dutyHandler(defaultDutyCycleIndex),
      _hasCustomDevEui(true)
{
  memcpy(_devEui, devEui, 8);
  memcpy(_appEui, appEui, 8);
  memcpy(_appKey, appKey, 16);
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
  if (payload->Port == PORT_CHANGE_INTERVAL)
  {
    if (payload->BufferSize > 0)
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
}

bool LualtekRAKRUI::setup()
{
  delay(1000); // Small stability delay
  _debugStream->println(F("--- Lualtek RAKRUI Init ---"));

  // 1. Restore Duty Cycle from Flash
  uint8_t savedIndex = _flash.readUplinkIndex(_dutyHandler.getCurrentIndex());
  // We update logic but don't save to flash (redundant) or start timers yet
  _dutyHandler.setCycle(savedIndex);

  // 2. Hardware Setup
  if (!api.system.lpm.set(1))
    return false;
  if (!api.lorawan.nwm.set())
    return false;

  if (!api.lorawan.appeui.set(_appEui, 8))
    return false;
  if (!api.lorawan.appkey.set(_appKey, 16))
    return false;

  if (_hasCustomDevEui)
  {
    if (!api.lorawan.deui.set(_devEui, 8))
      return false;
  }

  if (!api.lorawan.band.set(RAK_REGION_EU868))
    return false;
  if (!setClass(RAK_LORA_CLASS_A))
    return false;
  if (!api.lorawan.njm.set(RAK_LORA_OTAA))
    return false;

  return true;
}

bool LualtekRAKRUI::join(uint32_t attemptTimeoutMs)
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
      _debugStream->println(F("Err: Join Timeout"));
      return false;
    }

    api.lorawan.join();
    // Yield to system background tasks
    delay(5000);
  }

  // Post-Join Configuration
  api.lorawan.adr.set(true);
  api.lorawan.rety.set(1);
  api.lorawan.cfm.set(false);

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
