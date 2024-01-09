#include "LualtekRAK3172.h"

bool isDutyCycleIndex(unsigned int commandIndex)
{
  return commandIndex >= 0 && commandIndex <= sizeof(dutyCycleCommandTable) - 1;
}

bool isKeyEmpty(const uint8_t *array, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    if (array[i] != 0x00)
    {
      return false;
    }
  }
  return true;
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
    debugStream->println(F("Received downlink for changing duty cycle"));
    handleChangeDutyCycle(payload->Buffer[0]);
    api.system.timer.stop(RAK_TIMER_0);
    if (api.system.timer.start(RAK_TIMER_0, getUplinkInterval(), NULL) != true)
    {
      debugStream->println(F("Error starting timer"));
    }
    break;
  case DOWNLINK_ACTION_REJOIN_PORT:
    debugStream->println(F("Received downlink for rejoin. Rejoining..."));
    api.system.reboot();
    break;
  default:
    break;
  }
}

bool LualtekRAK3172::setup()
{
  delay(1500);
  debugStream->println(F("Lualtek RAK3172 Setup"));
  debugStream->println(F("------------------------"));
  // Setup duty cycle from EEPROM if available or use default
  uint8_t uplinkIntervalFlash[1];
  api.system.flash.get(0, uplinkIntervalFlash, 1);
  handleChangeDutyCycle(isDutyCycleIndex(uplinkIntervalFlash[0]) ? uplinkIntervalFlash[0] : defaultDutyCycleIndex);

  if (!api.system.lpm.set(1))
  {
    debugStream->println(F("LoRaWan Settings - set low power mode is incorrect!"));
    return false;
  }

  // https://docs.rakwireless.com/RUI3/LoRaWAN/#nwm
  if (!api.lorawan.nwm.set(1))
  {
    debugStream->println(F("LoRaWan Settings - set network working mode is incorrect!"));
    return false;
  }

  if (!api.lorawan.appeui.set(appEui, 8))
  {
    debugStream->println(F("LoRaWan Settings - set application EUI is incorrect!"));
    return false;
  }

  if (!api.lorawan.appkey.set(appKey, 16))
  {
    debugStream->println(F("LoRaWan Settings - set application key is incorrect!"));
    return false;
  }

  // Set the device EUI if available
  if (isKeyEmpty(devEui, 8))
  {
    api.lorawan.deui.get(devEui, 8);
  }

  if (!api.lorawan.deui.set(devEui, 8))
  {
    debugStream->println(F("LoRaWan Settings - set device EUI is incorrect!"));
    return false;
  }

  if (!api.lorawan.band.set(RAK_REGION_EU868))
  {
    debugStream->println(F("LoRaWan Settings - set band is incorrect!"));
    return false;
  }

  if (!setClass(RAK_LORA_CLASS_A))
  {
    return false;
  }

  // Set the network join mode to OTAA
  if (!api.lorawan.njm.set(RAK_LORA_OTAA))
  {
    debugStream->println(F("LoRaWan Settings - set network join mode is incorrect!"));
    return false;
  }

  return true;
}

bool LualtekRAK3172::join()
{
  // TODO: remove when RAK implments nbtrials
  // Set the data rate to 0 (SF12, BW125KHz) for joining
  // Then let adr handle the data rate
  api.lorawan.dr.set(0);

  delay(random(500, 8000));
  if (!api.lorawan.join())
  {
    debugStream->println(F("LoRaWan Settings - join fail!"));
    return false;
  }

  /** Wait for Join success */
  while (api.lorawan.njs.get() == 0)
  {
    debugStream->print(F("Wait for LoRaWAN join..."));
    api.lorawan.join();
    delay(5000);
  }

  if (!api.lorawan.adr.set(true))
  {
    debugStream->println(F("LoRaWan Settings - set adaptive data rate is incorrect!"));
  }

  if (!api.lorawan.rety.set(1))
  {
    debugStream->println(F("LoRaWan Settings - set retry times is incorrect!"));
  }

  // Disable confirmation mode
  if (!api.lorawan.cfm.set(false))
  {
    debugStream->println(F("LoRaWan Settings - set confirm mode is incorrect!"));
  }

  api.lorawan.daddr.get(assigned_dev_addr, 4);

  debugStream->printf("Duty cycle is %s\r\n", api.lorawan.dcs.get() ? "ON" : "OFF");                                                                     // Check Duty Cycle status
  debugStream->printf("Packet is %s\r\n", api.lorawan.cfm.get() ? "CONFIRMED" : "UNCONFIRMED");                                                          // Check Confirm status
  debugStream->printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]); // Check Device Address
  debugStream->printf("Uplink period is %ums\r\n", getUplinkInterval());                                                                                 // Check Uplink period
  debugStream->println(F(""));

  return true;
}

bool LualtekRAK3172::setClass(RAK_LORA_CLASS classType)
{
  if (!api.lorawan.deviceClass.set(classType))
  {
    debugStream->println(F("LoRaWan Settings - set device class is incorrect!"));
    return false;
  }

  return true;
}

bool LualtekRAK3172::setupTimers(void (*callback)())
{
  if (api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)callback, RAK_TIMER_PERIODIC) != true)
  {
    debugStream->println(F("LoRaWan Settings - Creating timer failed."));
    return false;
  }

  if (api.system.timer.start(RAK_TIMER_0, getUplinkInterval(), NULL) != true)
  {
    debugStream->println(F("LoRaWan Settings - Starting timer failed."));
    return false;
  }

  return true;
}

bool LualtekRAK3172::send(uint8_t size, uint8_t *data, uint8_t fPort)
{
  if (api.lorawan.send(size, data, fPort, false))
  {
    debugStream->println(F("Sending is requested"));
    return true;
  }

  debugStream->println(F("Sending failed"));
  return false;
}

int LualtekRAK3172::getUplinkInterval()
{
  return dutyCycleHandler.uplinkInterval;
}

int LualtekRAK3172::getBatteryVoltage()
{
  analogReadResolution(12);
  float max, ref;

  switch (udrv_adc_get_resolution())
  {
  case UDRV_ADC_RESOLUTION_6BIT:
  {
    max = 64.0;
    break;
  }
  case UDRV_ADC_RESOLUTION_8BIT:
  {
    max = 256.0;
    break;
  }
  case UDRV_ADC_RESOLUTION_10BIT:
  default:
  {
    max = 1024.0;
    break;
  }
  case UDRV_ADC_RESOLUTION_12BIT:
  {
    max = 4096.0;
    break;
  }
  case UDRV_ADC_RESOLUTION_14BIT:
  {
    max = 16384.0;
    break;
  }
  }

  switch (udrv_adc_get_mode())
  {
  case UDRV_ADC_MODE_DEFAULT:
  default:
  {
#ifdef rak11720
    ref = 2.0;
#else
    ref = 3.6;
#endif
    break;
  }
#ifdef rak11720
  case UDRV_ADC_MODE_1_5:
  {
    ref = 1.5;
    break;
  }
#else
  case UDRV_ADC_MODE_3_3:
  {
    ref = 3.3;
    break;
  }
  case UDRV_ADC_MODE_3_0:
  {
    ref = 3.0;
    break;
  }
  case UDRV_ADC_MODE_2_4:
  {
    ref = 2.4;
    break;
  }
  case UDRV_ADC_MODE_1_8:
  {
    ref = 1.8;
    break;
  }
  case UDRV_ADC_MODE_1_2:
  {
    ref = 1.2;
    break;
  }
#endif
  }

  int adc_value = analogRead(WB_A0);
  analogReadResolution(10);

  // 2.25 voltage divider calculated by tests
  return (ref * (((float)adc_value) / max) * 2.25f) * 1000;
}
