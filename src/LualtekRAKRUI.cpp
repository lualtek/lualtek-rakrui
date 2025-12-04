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

// Private helper to handle the LED feedback at startup
void LualtekRAKRUI::startupBlinkingFeedback()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SCL, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(SCL, HIGH);
  delay(1000);

  for (int i = 0; i < 5; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(SCL, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(SCL, LOW);
    delay(100);
  }
}

// Private helper to turn off LED feedback
void LualtekRAKRUI::turnOffBlinkingFeedback()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SCL, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(SCL, HIGH);
  delay(4000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SCL, LOW);
}

bool LualtekRAKRUI::executeJoinSequence(uint8_t limitPackets)
{
  const int8_t START_DR = 5;    // SF7 (Fastest)
  const int8_t MIN_DR = 0;      // SF12 (Slowest)
  const int RETRIES_PER_DR = 3; // 3 tries per DR

  int8_t currentDr = START_DR;
  int drAttempts = 0;
  int totalSent = 0;

  // Reset DR to start
  api.lorawan.dr.set(currentDr);
  _debugStream->println(F("Starting Join Sequence..."));

  while (totalSent < limitPackets)
  {
    // 1. Check if already joined (Checks success of PREVIOUS iteration)
    if (api.lorawan.njs.get() == 1)
      return true;

    // 2. Prepare Data Rate (Logic: 3 tries then lower speed)
    if (totalSent > 0)
    {
      drAttempts++;
      if (drAttempts >= RETRIES_PER_DR)
      {
        drAttempts = 0;
        currentDr--;

        // If we go below DR0, cycle back to top
        if (currentDr < MIN_DR)
          currentDr = START_DR;

        _debugStream->printf("Join: Lowering to DR%d\r\n", currentDr);
        api.lorawan.dr.set(currentDr);
      }
    }
    else
    {
      _debugStream->printf("Join: Starting at DR%d\r\n", currentDr);
    }

    // 3. Send Join Request
    api.lorawan.join();
    totalSent++;
    _debugStream->printf("Join: Packet Sent (%d/%d)\r\n", totalSent, limitPackets);

    // 4. Blocking Wait for RX1 (5s) and RX2 (6s)
    // We wait 8s to be safe.
    delay(8000);
  }

  // 5. Final Check
  // If the very last packet succeeded during the last delay(8000),
  // the loop exits because totalSent == limit, so we must check one last time.
  return (api.lorawan.njs.get() == 1);
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
    _debugStream->println(F("DL: Turn Off Magnet Request. Turning off..."));
    turnOffBlinkingFeedback();
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

  startupBlinkingFeedback();
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

bool LualtekRAKRUI::join(JoinBehavior behavior, uint8_t maxAttempts)
{
  _debugStream->println(F("LoRaWAN Join requested"));

  // CONSTANTS FOR LOGIC
  // DR5 to DR0 is 6 steps. 3 tries per step = 18 packets for a full sweep.
  const uint8_t FULL_SWEEP_PACKETS = 18;

  while (true)
  {
    // Determine how many packets to try in this cycle
    uint8_t limit = maxAttempts;

    // If JOIN_FOREVER, we ignore the user's maxAttempts and run a full sweep
    // (DR5 -> DR0) before sleeping.
    if (behavior == JOIN_FOREVER)
    {
      limit = FULL_SWEEP_PACKETS;
    }

    // Execute the sequence
    bool joined = executeJoinSequence(limit);

    // 1. Success
    if (joined)
    {
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

      uint8_t assigned_addr[4];
      api.lorawan.daddr.get(assigned_addr, 4);
      _debugStream->printf("Joined! DevAddr: %02X%02X%02X%02X\r\n",
                           assigned_addr[0], assigned_addr[1], assigned_addr[2], assigned_addr[3]);

      return true;
    }

    // 2. Failure Handling
    if (behavior == JOIN_ONCE)
    {
      _debugStream->printf("Err: Join failed after %d attempts. Giving up.\r\n", maxAttempts);
      return false;
    }

    // 3. JOIN_FOREVER Loop
    _debugStream->println(F("Err: Full join sweep failed. Deep sleep 2 mins, then retrying..."));

    // Deep sleep to save battery before trying the sequence again
    api.system.sleep.all(120000);
  }
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
