#ifndef _LualtekRAKRUI_H_
#define _LualtekRAKRUI_H_

#include <Arduino.h>
#include "DutyCycleHandler.h"
#include "SmartFlash.h"

enum DownlinkPort : uint8_t
{
  PORT_ACTION = 1,
  PORT_CHANGE_INTERVAL = 3,
  PORT_REJOIN = 10,
  PORT_TURN_OFF_MAGNET = 20
};

class LualtekRAKRUI
{
public:
  // Construct using the module DevEUI stored in flash
  LualtekRAKRUI(
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      uint8_t defaultDutyCycleIndex,
      Stream *debugStream);

  // Initialize peripherals, flash and duty-cycle state
  bool setup();

  // attemptTimeoutMs: How long to try joining before giving up (prevent blocking forever)
  bool join(uint32_t attemptTimeoutMs = 60000);

  // Switch between LoRaWAN device classes
  bool setClass(RAK_LORA_CLASS classType);
  // Register the periodic uplink callback and start timers
  bool setupTimers(void (*uplinkRoutine)());

  // RUI3 callback invoked whenever a downlink payload arrives
  void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload);

  // Send an uplink payload with the given size and port
  bool send(uint8_t dataSize, uint8_t *data, uint8_t fPort);
  // Return the current uplink interval derived from duty-cycle settings
  uint32_t getUplinkIntervalMs();

private:
  // Persist the new duty-cycle index and refresh timers accordingly
  void processDutyCycleChange(uint8_t newIndex);

  uint8_t _appEui[8];
  uint8_t _appKey[16];

  Stream *_debugStream;

  // Composition: These are members of the main class
  DutyCycleHandler _dutyHandler;
  SmartFlash _flash;
};

#endif
