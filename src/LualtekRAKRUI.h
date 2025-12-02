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
  LualtekRAKRUI(
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      uint8_t defaultDutyCycleIndex,
      Stream *debugStream);

  LualtekRAKRUI(
      const uint8_t devEui[8],
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      uint8_t defaultDutyCycleIndex,
      Stream *debugStream);

  bool setup();

  // attemptTimeoutMs: How long to try joining before giving up (prevent blocking forever)
  bool join(uint32_t attemptTimeoutMs = 60000);

  bool setClass(RAK_LORA_CLASS classType);
  bool setupTimers(void (*uplinkRoutine)());

  // RUI3 Callback signature
  void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload);

  bool send(uint8_t dataSize, uint8_t *data, uint8_t fPort);
  uint32_t getUplinkIntervalMs();

private:
  // Helper to process internal logic changes
  void processDutyCycleChange(uint8_t newIndex);

  uint8_t _devEui[8];
  uint8_t _appEui[8];
  uint8_t _appKey[16];
  bool _hasCustomDevEui;

  Stream *_debugStream;

  // Composition: These are members of the main class
  DutyCycleHandler _dutyHandler;
  SmartFlash _flash;
};

#endif
