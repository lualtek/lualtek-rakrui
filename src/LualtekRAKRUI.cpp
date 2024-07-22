#include "LualtekRAKRUI.h"

// LUT1_EV_Full Li-ION/LiPo scale up to 4.3v, 10 loops of sampling
// const int adcValues[] = {1583, 1626, 1668, 1709, 1750, 1791, 1832, 1872, 1911, 1950, 1989, 2027, 2065, 2103, 2140, 2177, 2214, 2252, 2285}; // ADC readings
// const float batteryValues[] = {2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4.0, 4.1, 4.2, 4.3}; // Corresponding battery values in volts

// LUT2_EV_Full Li-ION/LiPo scale up to 4.3v, single shot sampling
const int adcValues[] = {1549, 1592, 1635, 1675, 1715, 1754, 1792, 1829, 1866, 1901, 1936, 1969, 2004, 2038, 2071, 2105, 2139, 2173, 2205}; // ADC readings
const float batteryValues[] = {2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4.0, 4.1, 4.2, 4.3};              // Corresponding battery values in volts

/*
  * New LUT sampled on the combo RAK3172 + RAK19007

*/
// LUT_RAK19007_Full Li-ION/LiPo scale up to 4.3v, 10X sampling
const int adcValues_RAK19007[] = {1904, 1947, 1990, 2030, 2070, 2109, 2147, 2184, 2221, 2256, 2292, 2324, 2359, 2393, 2427, 2471, 2494, 2527, 2570}; // ADC readings
const float batteryValues_RAK19007[] = {2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4.0, 4.1, 4.2, 4.3};              // Corresponding battery values in volts

const int tableSize = sizeof(adcValues) / sizeof(adcValues[0]);
const int tableSize_RAK19007 = sizeof(adcValues_RAK19007) / sizeof(adcValues_RAK19007[0]);

bool isDutyCycleIndex(unsigned int commandIndex)
{
  return commandIndex >= 0 && commandIndex <= sizeof(dutyCycleCommandTable) - 1;
}

float convertToBatteryVoltage(int adcReading)
{
  for (int i = 0; i < tableSize - 1; ++i)
  {
    if (adcReading >= adcValues[i] && adcReading <= adcValues[i + 1])
    {
      // Linear interpolation to get the battery voltage
      float slope = (batteryValues[i + 1] - batteryValues[i]) / (adcValues[i + 1] - adcValues[i]);
      float batteryVoltage = batteryValues[i] + slope * (adcReading - adcValues[i]);
      return batteryVoltage;
    }
  }
  // Return the minimum battery voltage if the ADC reading is below the minimum in the table
  return batteryValues[0];
}

/*
 * To be used with getBatteryVoltage_10x()
 */
float convertToBatteryVoltage_RAK19007(int adcReading)
{
  for (int i = 0; i < tableSize - 1; ++i)
  {
    if (adcReading >= adcValues_RAK19007[i] && adcReading <= adcValues_RAK19007[i + 1])
    {
      // Linear interpolation to get the battery voltage
      float slope = (batteryValues_RAK19007[i + 1] - batteryValues_RAK19007[i]) / (adcValues_RAK19007[i + 1] - adcValues_RAK19007[i]);
      float batteryVoltage = batteryValues_RAK19007[i] + slope * (adcReading - adcValues_RAK19007[i]);
      return batteryVoltage;
    }
  }
  // Return the minimum battery voltage if the ADC reading is below the minimum in the table
  return batteryValues_RAK19007[0];
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

LualtekRAKRUI::LualtekRAKRUI(
    const uint8_t appEui[8],
    const uint8_t appKey[16],
    lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
    Stream *debugStream) : debugStream(debugStream),
                           dutyCycleHandler(dutyCycleIndex)
{
  memcpy(this->appEui, appEui, 8);
  memcpy(this->appKey, appKey, 16);
}

LualtekRAKRUI::LualtekRAKRUI(
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

void LualtekRAKRUI::handleChangeDutyCycle(int commandIndex)
{
  dutyCycleHandler.changeDutyCycle(commandIndex);
}

void LualtekRAKRUI::onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload)
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

bool LualtekRAKRUI::setup()
{
  delay(1500);
  debugStream->println(F("Lualtek RAKRUI Setup"));
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
  if (!api.lorawan.nwm.set())
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

bool LualtekRAKRUI::join()
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

bool LualtekRAKRUI::setClass(RAK_LORA_CLASS classType)
{
  if (!api.lorawan.deviceClass.set(classType))
  {
    debugStream->println(F("LoRaWan Settings - set device class is incorrect!"));
    return false;
  }

  return true;
}

bool LualtekRAKRUI::setupTimers(void (*callback)())
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

bool LualtekRAKRUI::send(uint8_t size, uint8_t *data, uint8_t fPort)
{
  if (api.lorawan.send(size, data, fPort, false))
  {
    debugStream->println(F("Sending is requested"));
    return true;
  }

  debugStream->println(F("Sending failed"));
  return false;
}

int LualtekRAKRUI::getUplinkInterval()
{
  return dutyCycleHandler.uplinkInterval;
}

int LualtekRAKRUI::getBatteryVoltage()
{
  analogReadResolution(12);
  int adc_value = analogRead(WB_A0);
  analogReadResolution(10);
  return convertToBatteryVoltage(adc_value) * 1000;
}

/*
 * New function that reads the ADC multiple times to avoid underestimation of voltage
 */
int LualtekRAKRUI::getBatteryVoltage_10x()
{ // USE with LUT_RAK19007

  uint16_t adc_value = 0;

  uint16_t sum = 0;
  analogReadResolution(12);
  for (int i = 0; i < 10; i++)
  {
    sum += analogRead(WB_A0);
    delay(5);
  }
  analogReadResolution(10);
  adc_value = sum / 10;
  return convertToBatteryVoltage_RAK19007(adc_value) * 1000;
}
