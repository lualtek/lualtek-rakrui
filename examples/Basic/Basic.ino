#include <Arduino.h>
#include "LualtekRAKRUI.h"

// ==========================================
// 1. Credentials
// ==========================================
// Replace these with your keys from The Things Stack or your Network Server
const uint8_t appEui[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t appKey[16] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x05, 0x66, 0x12, 0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x05, 0x66, 0x12};

// ==========================================
// 2. Global Objects
// ==========================================
// Initialize the library with:
// - AppEUI, AppKey
// - Default Duty Cycle (e.g., MINUTES_20)
// - Pointer to Serial for debug output
LualtekRAKRUI node(appEui, appKey, MINUTES_20, &Serial);

// ==========================================
// 3. Uplink Routine (Timer Callback)
// ==========================================
// This function is called automatically by the library's internal timer
void sendSensorData()
{
  Serial.println(F("--- Timer Triggered: Preparing Packet ---"));

  // Simulate reading a sensor (e.g., Temperature)
  uint8_t temperature = 25;
  uint8_t humidity = 60;

  // Create a payload
  uint8_t payload[2];
  payload[0] = temperature;
  payload[1] = humidity;

  // Send the data on Port 1
  if (node.send(sizeof(payload), payload, 1))
  {
    Serial.println(F("Packet Queued Successfully"));
  }
  else
  {
    Serial.println(F("Error Queuing Packet"));
  }
}

// ==========================================
// 4. Downlink Callback
// ==========================================
// This function triggers when the Radio receives data
void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *data)
{
  Serial.printf("Downlink Received on Port %d\r\n", data->Port);

  // CRITICAL: Pass the data to the library first.
  // The library checks for Port 3 (Duty Cycle) and Port 10 (Reboot).
  node.onDownlinkReceived(data);

  // Handle your custom ports here
  if (data->Port == 1)
  {
    Serial.println(F("Custom command received on Port 1"));
    // Add your logic here (e.g., toggle a relay)
  }
}

// ==========================================
// 5. Setup
// ==========================================
void setup()
{
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(2000); // Wait for PC connection
  Serial.println(F("=== Lualtek Node Starting ==="));

  // A. Initialize Library Hardware Settings
  if (!node.setup())
  {
    Serial.println(F("Critical Error: Hardware Setup Failed"));
    while (1)
      ; // Halt
  }

  // B. Register the Downlink Callback with RUI3
  api.lorawan.registerRecvCallback(onDownlinkReceived);

  // C. Join the Network
  // We pass a 60-second timeout. If it fails, we can decide what to do.
  if (!node.join(60000))
  {
    Serial.println(F("Join Failed or Timed Out. Retrying later..."));
    // In a real scenario, you might deep sleep here and reset later
  }
  else
  {
    Serial.println(F("Successfully Joined Network"));
  }

  // D. Start the Automated Timer
  // This will start calling 'sendSensorData' at the interval
  // defined by the DutyCycleHandler (default or loaded from Flash)
  if (!node.setupTimers(sendSensorData))
  {
    Serial.println(F("Error: Could not setup timers"));
  }
}

// ==========================================
// 6. Loop
// ==========================================
void loop()
{
  // RUI3 is event-driven.
  // We put the system to sleep to save battery.
  // The Timer and Radio Interrupts will wake the CPU automatically.
  api.system.sleep.all();
}
