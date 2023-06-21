# LualtekRAK3172 Arduino Library

The LualtekRAK3172 library is an opinionated wrapper around the RUI3 API by RAKWireless. It simplifies the usage of the RAK3172 LoRa module in Arduino projects, providing a convenient interface for configuring and interacting with the module.

For more information about the RUI3 API, refer to the [RUI3 Documentation](https://docs.rakwireless.com/RUI3/#overview).

For more information about the RAK3172 module, refer to the [RAK3172 Documentation](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3172-Evaluation-Board/Overview/#product-description).

## Features

- Simplified setup and configuration of the RAK3172 LoRa module.
- Handling of downlink messages for changing duty cycle.
- Timer-based scheduling for uplink transmissions.
- Support for sending data packages to the LoRaWAN network.

## Installation

You can install the LualtekRAK3172 library through the Arduino Library Manager or manually as a ZIP file.

### Arduino Library Manager (Recommended)

1. Open the Arduino IDE.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries**.
3. In the Library Manager, search for "LualtekRAK3172".
4. Click on the LualtekRAK3172 library and click the **Install** button.

### Manual Installation

1. Download the LualtekRAK3172 library as a ZIP file from the [GitHub repository](https://github.com/username/repo).
2. In the Arduino IDE, navigate to **Sketch** -> **Include Library** -> **Add .ZIP Library**.
3. Select the downloaded ZIP file of the library and click **Open**.

## Usage

### Prerequisites

- RAK3172 LoRa module.
- Knowledge of the RUI3 API by RAKWireless.

### Library Initialization

To begin using the LualtekRAK3172 library, include the library header at the beginning of your sketch:

```cpp
#include <LualtekRAK3172.h>
```

Next, create an instance of the `LualtekRAK3172` class:

```cpp
const uint8_t appEui[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t appKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// With 20 minutes duty cycle
LualtekRAK3172 lualtek(appEui, appKey, MINUTES_20_COMMAND_INDEX);
```

Replace `appEui` and `appKey` with your LoRaWAN application EUI and application key respectively.

### Configuration and Setup

To configure and set up the RAK3172 LoRa module, call the `setup()` method:

```cpp
bool success = lualtek.setup();
if (success) {
  // Setup successful, proceed with further configuration or operations
} else {
  // Setup failed, handle the error
}
```

### Joining the LoRaWAN

 Network

To join the LoRaWAN network using OTAA, call the `join()` method:

```cpp
bool success = lualtek.join();
if (success) {
  // Join successful, proceed with data transmissions
} else {
  // Join failed, handle the error
}
```

### Sending Data

To send data packages to the LoRaWAN network, use the `send()` method:

```cpp
uint8_t data[] = {0x01, 0x02, 0x03};
uint8_t size = sizeof(data);
uint8_t fPort = 1;

bool success = lualtek.send(size, data, fPort);
if (success) {
  // Data send request successful
} else {
  // Data send request failed, handle the error
}
```

Replace `data` with your payload data, `size` with the size of the payload, and `fPort` with the LoRaWAN port number.

### Handling Downlink Messages

To handle downlink messages, register the `onDownlinkReceived()` callback with the RUI3 API using `api.lorawan.registerRecvCallback()`:

```cpp
void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload) {
  // This will handle the downlink message for changing duty cycle or rebooting
  lltek.onDownlinkReceived(data);

  // Handle the downlink message based on payload properties
}

// Register the callback
api.lorawan.registerRecvCallback(onDownlinkReceived);
```


#### Supported Downlink Commands

The LualtekRAK3172 library supports the following downlink commands:

- **Change Duty Cycle**: To change the duty cycle, send a downlink message with `fPort` set to `DOWNLINK_ACTION_CHANGE_INTERVAL_PORT` (3) and the payload as follows:

  - `0`: Set duty cycle to 60 minutes.
  - `1`: Set duty cycle to 40 minutes.
  - `2`: Set duty cycle to 30 minutes.
  - `3`: Set duty cycle to 20 minutes.
  - `4`: Set duty cycle to 15 minutes.
  - `5`: Set duty cycle to 10 minutes.
  - `6`: Set duty cycle to 5 minutes.
  - `7`: Set duty cycle to 1 minute.

- **Reboot**: To reboot the module, send a downlink message with `fPort` set to `DOWNLINK_ACTION_REJOIN_PORT` (10) and an empty payload (0 bytes).

For more information about registering the callback, refer to the [RUI3 Documentation](https://docs.rakwireless.com/RUI3/LoRaWAN/#registerrecvcallback).

### Scheduling Uplink Transmissions

To schedule uplink transmissions at regular intervals, use the `setupTimers()` method:

```cpp
bool success = lualtek.setupTimers(callbackFunction);
if (success) {
  // Timer setup successful, callbackFunction will be invoked at specified intervals
} else {
  // Timer setup failed, handle the error
}
```

Replace `callbackFunction` with the function that should be called at the specified intervals.

## License

This project is licensed under the [MIT License](LICENSE).
