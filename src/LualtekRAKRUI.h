#ifndef _LualtekRAKRUI_H_
#define _LualtekRAKRUI_H_

#include <Arduino.h>
#include "DutyCycleHandler.h"
#include "SmartFlash.h"
#include "MagnetHandler.h"

enum DownlinkPort : uint8_t
{
  PORT_ACTION = 1,
  PORT_CHANGE_INTERVAL = 3,
  PORT_REJOIN = 10,
  PORT_TURN_OFF_MAGNET = 20
};

enum JoinBehavior : uint8_t
{
  JOIN_FOREVER = 0,
  JOIN_ONCE = 1
};

enum PowerModeKind : uint8_t
{
  POWER_MODE_MAGNET = 0,
  POWER_MODE_CONNECTOR = 1
};

class LualtekRAKRUI
{
public:
  // Construct using the module DevEUI stored in flash
  LualtekRAKRUI(
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      uint8_t defaultDutyCycleIndex,
      PowerModeKind powerMode,
      Stream *debugStream);

  /** Initialize peripherals, flash and duty-cycle state
   * NOTE: This should be called after debugSerial.begin() and a delay to allow the serial port to stabilize
   */
  bool setup();

  /**
   * @brief Attempts to join the network.
   * @param behavior JOIN_ONCE or JOIN_FOREVER.
   * @param maxAttempts Max number of TX packets to send if behavior is JOIN_ONCE.
   */
  bool join(JoinBehavior behavior = JOIN_FOREVER, uint8_t maxAttempts = 18);

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
  void startupBlinkingFeedback();
  void turnOffBlinkingFeedback();

  // Returns true if joined, false if limit reached without join
  bool executeJoinSequence(uint8_t limitPackets);

  uint8_t _appEui[8];
  uint8_t _appKey[16];
  PowerModeKind _powerMode;

  Stream *_debugStream;

  // Composition: These are members of the main class
  DutyCycleHandler _dutyHandler;
  SmartFlash _flash;
  MagnetHandler _magnetHandler;
};

#endif
