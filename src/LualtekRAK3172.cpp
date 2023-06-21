#include "LualtekRAK3172.h"

bool isDutyCycleIndex(unsigned int commandIndex)
{
  return commandIndex >= 0 && commandIndex <= sizeof(dutyCycleCommandTable) - 1;
}

LualtekRAK3172::LualtekRAK3172(
    const uint8_t appEui[8],
    const uint8_t appKey[16],
    lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
    Stream *debugStream) : debugStream(debugStream),
                           dutyCycleHandler(dutyCycleIndex)
{
  memcpy(this->appEui, appEui, 8);
  memcpy(this->appKey, appKey, 16);
}

LualtekRAK3172::LualtekRAK3172(
    const uint8_t devEui[8],
    const uint8_t appEui[8],
    const uint8_t appKey[16],
    lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
    Stream *debugStream) : debugStream(debugStream),
                           dutyCycleHandler(dutyCycleIndex)
{
  memcpy(this->devEui, devEui, 8);
  memcpy(this->appEui, appEui, 8);
  memcpy(this->appKey, appKey, 16);
}

void LualtekRAK3172::handleChangeDutyCycle(int commandIndex)
{
  dutyCycleHandler.changeDutyCycle(commandIndex);
}

void LualtekRAK3172::onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload)
{
  switch (payload->Port)
  {
  case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
    debugStream->println("Received downlink for changing duty cycle");
    handleChangeDutyCycle(payload->Buffer[0]);
    api.system.timer.stop(RAK_TIMER_0);
    if (api.system.timer.start(RAK_TIMER_0, getUplinkInterval(), NULL) != true) {
      debugStream->println("Error starting timer");
    }
    break;
  case DOWNLINK_ACTION_REJOIN_PORT:
    debugStream->println("Received downlink for rejoin. Rejoining...");
    api.system.reboot();
    break;
  default:
    break;
  }
}

bool LualtekRAK3172::setup()
{
  delay(1500);
  debugStream->println("Lualtek RAK3172 Setup");
  debugStream->println("------------------------");
  // Setup duty cycle from EEPROM if available or use default
  uint8_t uplinkIntervalFlash[1];
  api.system.flash.get(0, uplinkIntervalFlash, 1);
  handleChangeDutyCycle(isDutyCycleIndex(uplinkIntervalFlash[0]) ? uplinkIntervalFlash[0] : defaultDutyCycleIndex);

  if (!api.system.lpm.set(1))
  {
    debugStream->printf("LoRaWan Settings - set low power mode is incorrect! \r\n");
    return false;
  }

  // https://docs.rakwireless.com/RUI3/LoRaWAN/#nwm
  if (!api.lorawan.nwm.set(1))
  {
    debugStream->printf("LoRaWan Settings - set network working mode is incorrect! \r\n");
    return false;
  }

  if (!api.lorawan.appeui.set(appEui, 8))
  {
    debugStream->printf("LoRaWan Settings - set application EUI is incorrect! \r\n");
    return false;
  }

  if (!api.lorawan.appkey.set(appKey, 16))
  {
    debugStream->printf("LoRaWan Settings - set application key is incorrect! \r\n");
    return false;
  }

  // Set the device EUI if available
  if (devEui == NULL)
  {
    api.lorawan.deui.get(devEui, 8);
  }

  if (!api.lorawan.deui.set(devEui, 8))
  {
    debugStream->printf("LoRaWan Settings - set device EUI is incorrect! \r\n");
    return false;
  }

  if (!api.lorawan.band.set(RAK_REGION_EU868))
  {
    debugStream->printf("LoRaWan Settings - set band is incorrect! \r\n");
    return false;
  }

  if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A))
  {
    debugStream->printf("LoRaWan Settings - set device class is incorrect! \r\n");
    return false;
  }

  // Set the network join mode to OTAA
  if (!api.lorawan.njm.set(RAK_LORA_OTAA))
  {
    debugStream->printf("LoRaWan Settings - set network join mode is incorrect! \r\n");
    return false;
  }

  return true;
}

bool LualtekRAK3172::join()
{
  delay(random(500, 8000));
  if (!api.lorawan.join())
  {
    debugStream->printf("LoRaWan Settings - join fail! \r\n");
    return false;
  }

  /** Wait for Join success */
  while (api.lorawan.njs.get() == 0)
  {
    debugStream->print("Wait for LoRaWAN join...");
    api.lorawan.join();
    delay(10000);
  }

  if (!api.lorawan.adr.set(true))
  {
    debugStream->printf("LoRaWan Settings - set adaptive data rate is incorrect! \r\n");
    return false;
  }

  if (!api.lorawan.rety.set(1))
  {
    debugStream->printf("LoRaWan Settings - set retry times is incorrect! \r\n");
    return false;
  }

  // Disable confirmation mode
  if (!api.lorawan.cfm.set(false))
  {
    debugStream->printf("LoRaWan Settings - set confirm mode is incorrect! \r\n");
    return false;
  }

  api.lorawan.daddr.get(assigned_dev_addr, 4);

  debugStream->printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");                                                                     // Check Duty Cycle status
  debugStream->printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED");                                                          // Check Confirm status
  debugStream->printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]); // Check Device Address
  debugStream->printf("Uplink period is %ums\r\n", getUplinkInterval());                                                                                     // Check Uplink period
  debugStream->println("");

  return true;
}

bool LualtekRAK3172::setupTimers(void (*callback)())
{
  if (api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)callback, RAK_TIMER_PERIODIC) != true)
  {
    debugStream->printf("LoRaWan Settings - Creating timer failed.\r\n");
    return false;
  }

  if (api.system.timer.start(RAK_TIMER_0, getUplinkInterval(), NULL) != true)
  {
    debugStream->printf("LoRaWan Settings - Starting timer failed.\r\n");
    return false;
  }

  return true;
}

bool LualtekRAK3172::send(uint8_t size, uint8_t *data, uint8_t fPort)
{
  /** Send the data package */
  if (api.lorawan.send(size, data, fPort, false))
  {
    debugStream->println("Sending is requested");
  }
  else
  {
    debugStream->println("Sending failed");
  }

  return true;
}

int LualtekRAK3172::getUplinkInterval()
{
  return dutyCycleHandler.uplinkInterval;
}

Stream &LualtekRAK3172::getDummyDebugStream()
{
  static Stream *dummyDebugStream = nullptr;
  if (dummyDebugStream == nullptr)
  {
    class DummyStream : public Stream
    {
    public:
      int available() { return 0; }
      int read() { return -1; }
      int peek() { return -1; }
      void flush() {}
      void printf(const char *, ...) {}
      size_t write(uint8_t) { return 1; }
    };
    static DummyStream instance;
    dummyDebugStream = &instance;
  }
  return *dummyDebugStream;
}
