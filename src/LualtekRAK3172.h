#ifndef _LUALTEKRAK3172_H_
#define _LUALTEKRAK3172_H_

#include <Arduino.h>
#include "DutyCycleHandler.h"

// Enumeration for LoRaWAN downlink command ports
enum lualtek_downlink_command_ports_t
{
  DOWNLINK_ACTION_COMMAND_PORT = 1,
  DOWNLINK_ACTION_CHANGE_INTERVAL_PORT = 3,
  DOWNLINK_ACTION_REJOIN_PORT = 10
};

/**
 * @brief Represents the Lualtek RAK3172 LoRaWAN module for Arduino.
 */
class LualtekRAK3172
{
public:
  /**
   * @brief Creates an instance of LualtekRAK3172.
   * @param appEui The application EUI.
   * @param appKey The application key.
   * @param dutyCycleIndex The duty cycle index.
   * @param debugStream Debug stream.
   */
  LualtekRAK3172(
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
      Stream *debugStream);

  /**
   * @brief Creates an instance of LualtekRAK3172 with a specified device EUI.
   * @param devEui The device EUI.
   * @param appEui The application EUI.
   * @param appKey The application key.
   * @param dutyCycleIndex The duty cycle index.
   * @param debugStream Debug stream.
   */
  LualtekRAK3172(
      const uint8_t devEui[8],
      const uint8_t appEui[8],
      const uint8_t appKey[16],
      lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
      Stream *debugStream);

  /**
   * @brief Initializes the LoRaWAN module.
   * @return True if the setup is successful; otherwise, false.
   */
  bool setup();

  /**
   * @brief Joins the LoRaWAN network.
   * @return True if the join is successful; otherwise, false.
   */
  bool join();

  /**
   * @brief Sets the LoRaWAN device class.
   * @param classType The device class type.
   * @return True if successful; otherwise, false.
   */
  bool setClass(RAK_LORA_CLASS classType);

  /**
   * @brief Sets up timers for periodic tasks.
   * @param uplinkRoutine The callback function for the periodic task.
   * @return True if successful; otherwise, false.
   */
  bool setupTimers(void (*uplinkRoutine)());

  /**
   * @brief Handles downlink messages received from the LoRaWAN network.
   * @param payload The received payload.
   */
  void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload);

  /**
   * @brief Handles a change in duty cycle based on the command index.
   * @param commandIndex The command index for duty cycle change.
   */
  void handleChangeDutyCycle(int commandIndex);

  /**
   * @brief Sends a message over the LoRaWAN network.
   * @param dataSize The size of the message.
   * @param data The message data.
   * @param fPort The port for the message.
   * @return True if the send is successful; otherwise, false.
   */
  bool send(uint8_t dataSize, uint8_t *data, uint8_t fPort);

  /**
   * @brief Gets the uplink interval.
   * @return The uplink interval in milliseconds.
   */
  int getUplinkInterval();

  /**
   * @brief Gets the battery voltage in mV.
   * @return The battery level as an integer.
   */
  int getBatteryVoltage();

private:
  uint8_t devEui[8];
  uint8_t appEui[8];
  uint8_t appKey[16];
  uint8_t assigned_dev_addr[4] = {0};
  lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex;
  unsigned long uplinkInterval;

  Stream *debugStream;
  DutyCycleHandler dutyCycleHandler;
};

#endif
