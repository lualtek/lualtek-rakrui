/**
 * WARNING: currently not supported by PlatformIO.
 * Reference https://docs.rakwireless.com/Product-Categories/WisBlock/RAK3372/Quickstart/#software-initial-guide
 *
 * NOTE: this example uses a custom IO expander that is powered on/off by WB_IO2: https://github.com/piecol/Wisblock_IO_extention_10x23
 **/
#include "LualtekRAKRUI.h"
#include <Adafruit_Sensor.h>
#include <Tiny_BME280.h> // https://github.com/jasonacox/Tiny_BME280_Library

Tiny_BME280 bme;

void uplinkRoutine();

// De-comment this to activate custom DEVEUI (WARNING: not tested)
uint8_t DEVEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t APPEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t APPKEY[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define debugSerial Serial

#if defined(CUSTOM_DEVEUI)
LualtekRAKRUI lltek(DEVEUI, APPEUI, APPKEY, MINUTES_20_COMMAND_INDEX, &debugSerial);
#else
LualtekRAKRUI lltek(APPEUI, APPKEY, MINUTES_20_COMMAND_INDEX, &debugSerial);
#endif

#define POWER_UP_SENSOR_PIN (WB_IO2)
// BME280 tts-formatter by port
#define F_PORT 3

int temperature = 0;
int humidity = 0;
int pressure = 0;
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

bool bmeInit()
{
  if (!bme.begin(BME280_ADDRESS_ALTERNATE))
  {
    debugSerial.println("Could not find a valid BME280 sensor, check wiring!");
    return false;
  }
  delay(200);
  return true;
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
  bmeInit();
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure();

  debugSerial.printf("Sensor values read: \r\n");
  debugSerial.printf("Temperature = %d *C \r\n", temperature);
  debugSerial.printf("Humidity = %d %% \r\n", humidity);
  debugSerial.printf("Pressure = %.2f hPa \r\n", pressure / 100.0F);
}

void uplinkRoutine()
{
  powerSensor(HIGH);
  readSensor();
  powerSensor(LOW);

  voltage = api.system.bat.get() * 1000;

  dataSize = 0;
  payloadData[dataSize++] = highByte(temperature);
  payloadData[dataSize++] = lowByte(temperature);
  payloadData[dataSize++] = highByte(humidity);
  payloadData[dataSize++] = lowByte(humidity);
  payloadData[dataSize++] = (byte)((pressure & 0xFF000000) >> 24);
  payloadData[dataSize++] = (byte)((pressure & 0x00FF0000) >> 16);
  payloadData[dataSize++] = (byte)((pressure & 0x0000FF00) >> 8);
  payloadData[dataSize++] = (byte)((pressure & 0X000000FF));
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
