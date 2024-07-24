#ifndef _DUTYCYCLEHANDLER_H_
#define _DUTYCYCLEHANDLER_H_

#include <Arduino.h>

enum lualtek_dowlink_command_dutycycle_index_t
{
  MINUTES_60_COMMAND_INDEX = 0,
  MINUTES_40_COMMAND_INDEX = 1,
  MINUTES_30_COMMAND_INDEX = 2,
  MINUTES_20_COMMAND_INDEX = 3,
  MINUTES_15_COMMAND_INDEX = 4,
  MINUTES_10_COMMAND_INDEX = 5,
  MINUTES_5_COMMAND_INDEX = 6,
  MINUTES_1_COMMAND_INDEX = 7,
  MINUTES_720_COMMAND_INDEX = 8,
  MINUTES_1440_COMMAND_INDEX = 9
};

enum lualtek_dutycycle_ms_t
{
  MINUTES_60_IN_MILLISECONDS = 3600000,
  MINUTES_40_IN_MILLISECONDS = 2400000,
  MINUTES_30_IN_MILLISECONDS = 1800000,
  MINUTES_20_IN_MILLISECONDS = 1200000,
  MINUTES_15_IN_MILLISECONDS = 900000,
  MINUTES_10_IN_MILLISECONDS = 600000,
  MINUTES_5_IN_MILLISECONDS = 300000,
  MINUTES_1_IN_MILLISECONDS = 60000,
  // 12 hours
  MINUTES_720_IN_MILLISECONDS = 43200000,
  // 24 hours
  MINUTES_1440_IN_MILLISECONDS = 86400000
};

const lualtek_dutycycle_ms_t dutyCycleCommandTable[] = {
    MINUTES_60_IN_MILLISECONDS,
    MINUTES_40_IN_MILLISECONDS,
    MINUTES_30_IN_MILLISECONDS,
    MINUTES_20_IN_MILLISECONDS,
    MINUTES_15_IN_MILLISECONDS,
    MINUTES_10_IN_MILLISECONDS,
    MINUTES_5_IN_MILLISECONDS,
    MINUTES_1_IN_MILLISECONDS,
    MINUTES_720_IN_MILLISECONDS,
    MINUTES_1440_IN_MILLISECONDS};

class DutyCycleHandler
{
public:
  DutyCycleHandler(lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex);
  void changeDutyCycle(int commandIndex);
  unsigned long uplinkInterval;

private:
  mutable unsigned long previousMillis;
  lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex;
  static bool isDutyCycleIndex(unsigned int commandIndex);
};

#endif
