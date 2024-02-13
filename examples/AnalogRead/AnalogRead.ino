/**
 * WARNING: currently not supported by PlatformIO.
 * Reference https://docs.rakwireless.com/Product-Categories/WisBlock/RAK3372/Quickstart/#software-initial-guide
 *
 * NOTE: this example uses a custom IO expander that is powered on/off by WB_IO2: https://github.com/piecol/Wisblock_IO_extention_10x23
 **/
#include "LualtekRAKRUI.h"

void uplinkRoutine();

// De-comment this to activate custom DEVEUI
// #define CUSTOM_DEVEUI
// uint8_t DEVEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t APPKEY[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define debugSerial Serial

#if defined(CUSTOM_DEVEUI)
LualtekRAKRUI lltek(DEVEUI, APPEUI, APPKEY, MINUTES_20_COMMAND_INDEX, &debugSerial);
#else
LualtekRAKRUI lltek(APPEUI, APPKEY, MINUTES_20_COMMAND_INDEX, &debugSerial);
#endif

#define POWER_UP_SENSOR_PIN (WB_IO2)
#define F_PORT 1

int analogValue = 0;
int voltage = 0;

uint8_t dataSize = 0;
/** Packet buffer for sending */
uint8_t payloadData[64] = {0};

void powerSensor(int KIND)
{
  pinMode(POWER_UP_SENSOR_PIN, OUTPUT);
  digitalWrite(POWER_UP_SENSOR_PIN, KIND);
  delay(2000);
}

void recvCallback(SERVICE_LORA_RECEIVE_T *data)
{
  if (data->BufferSize <= 0)
  {
    return;
  }

  lltek.onDownlinkReceived(data);
  uplinkRoutine();
}

void setup()
{
  // WARNING: this should be ON always or the board will not be upgradable
  debugSerial.begin(115200, RAK_AT_MODE);
  delay(4000);

  if (!lltek.setup())
  {
    debugSerial.println("Lualtek RAKRUI setup failed!");
    return;
  }

  if (!lltek.join())
  {
    debugSerial.println("Lualtek RAKRUI join failed!");
    return;
  }

  api.lorawan.registerRecvCallback(recvCallback);
  uplinkRoutine();

  if (!lltek.setupTimers(*uplinkRoutine))
  {
    debugSerial.println("Lualtek RAKRUI setup timers failed!");
    return;
  }
}

void readSensor()
{
  analogValue = analogRead(WB_A1);

  debugSerial.printf("Analog read: \r\n");
  debugSerial.printf("Value = %d \r\n", analogValue);
}

void uplinkRoutine()
{
  powerSensor(HIGH);
  readSensor();
  powerSensor(LOW);

  voltage = lltek.getBatteryVoltage();

  dataSize = 0;
  payloadData[dataSize++] = highByte(analogValue);
  payloadData[dataSize++] = lowByte(analogValue);
  payloadData[dataSize++] = highByte(voltage);
  payloadData[dataSize++] = lowByte(voltage);
  payloadData[dataSize++] = highByte((int)(lltek.getUplinkInterval() / 1000));
  payloadData[dataSize++] = lowByte((int)(lltek.getUplinkInterval() / 1000));

  debugSerial.println("Data Packet:");
  for (int i = 0; i < dataSize; i++)
  {
    debugSerial.printf("0x%02X ", payloadData[i]);
  }
  debugSerial.println("");

  lltek.send(dataSize, (uint8_t *)&payloadData, F_PORT);
}

void loop()
{
  /* Destroy this busy loop and use timer to do what you want instead,
   * so that the system thread can auto enter low power mode by api.system.lpm.set(1); */
  api.system.scheduler.task.destroy();
}
