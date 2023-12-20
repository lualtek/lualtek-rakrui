#ifndef _LUALTEKRAK3172_H_
#define _LUALTEKRAK3172_H_

#include <Arduino.h>
#include "DutyCycleHandler.h"

enum lualtek_downlink_command_ports_t
{
  DOWNLINK_ACTION_COMMAND_PORT = 1,
  DOWNLINK_ACTION_CHANGE_INTERVAL_PORT = 3,
  DOWNLINK_ACTION_REJOIN_PORT = 10
};

class LualtekRAK3172
{
public:
  LualtekRAK3172(
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
      Stream *debugStream = nullptr);
  LualtekRAK3172(
      const uint8_t devEui[8],
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
      Stream *debugStream = nullptr);

  bool setup();
  bool join();
  bool setClass(RAK_LORA_CLASS classType);
  // Setup time passing referenc callback for uplinkRoutine
  bool setupTimers(void (*uplinkRoutine)());
  void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload);
  void handleChangeDutyCycle(int commandIndex);
  bool send(uint8_t dataSize, uint8_t *data, uint8_t fPort);
  int getUplinkInterval();

private:
  uint8_t devEui[8];
  uint8_t appEui[8];
  uint8_t appKey[16];
  uint8_t assigned_dev_addr[4] = {0};
  lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex;
  unsigned long uplinkInterval;

  Stream *debugStream;

  // Returns a reference to a dummy debug stream (that doesn't print anything)
  static Stream &getDummyDebugStream();
  DutyCycleHandler dutyCycleHandler;
};

#endif
