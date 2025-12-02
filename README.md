# LualtekRAKRUI Arduino Library

The LualtekRAKRUI Arduino library is an opinionated wrapper around the RUI3 API by RAKWireless. It simplifies the usage of the RAK RUI3 LoRa module in Arduino projects, providing a convenient interface for configuring and interacting with the module.

For more information about the RUI3 API, refer to the [RUI3 Documentation](https://docs.rakwireless.com/RUI3/#overview).

## Features

- Simplified setup and configuration of the RAK RUI3 LoRa module.
- Handling of downlink messages for changing duty cycle.
- **Non-blocking** join attempts with timeouts.
- Timer-based scheduling for uplink transmissions.
- Integrated Flash memory management for setting persistence.
- Support for sending data packages to the LoRaWAN network.

## Installation

You can install the LualtekRAKRUI library through the Arduino Library Manager or manually as a ZIP file.

### Arduino Library Manager (Recommended)

1. Open the Arduino IDE.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries**.
3. In the Library Manager, search for "LualtekRAKRUI".
4. Click on the LualtekRAKRUI library and click the **Install** button.

### Manual Installation

1. Download the LualtekRAKRUI library as a ZIP file from the [GitHub repository](https://github.com/username/repo).
2. In the Arduino IDE, navigate to **Sketch** -> **Include Library** -> **Add .ZIP Library**.
3. Select the downloaded ZIP file of the library and click **Open**.

## Usage

### Prerequisites

- RAK with RUI3 LoRa module.
- Knowledge of the RUI3 API by RAKWireless.

### Library Initialization

To begin using the LualtekRAKRUI library, include the library header at the beginning of your sketch:

```cpp
#include <LualtekRAKRUI.h>
```

Next, create an instance of the `LualtekRAKRUI` class. You must pass the Serial interface for debug output:

```cpp
const uint8_t appEui[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t appKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Initialize with AppEUI, AppKey, Default Duty Cycle, and Debug Stream
// Example: Default 20 minutes duty cycle
LualtekRAKRUI lualtek(appEui, appKey, MINUTES_20, &Serial);
```

### Configuration and Setup

To configure and set up the RAK with RUI3 LoRa module, call the `setup()` method inside your Arduino `setup()`:

```cpp
void setup() {
  Serial.begin(115200);

  bool success = lualtek.setup();
  if (success) {
    // Setup successful
  } else {
    // Setup failed, handle the error
  }
}
```

### Joining the LoRaWAN Network

To join the LoRaWAN network using OTAA, call the `join()` method. You can optionally pass a timeout in milliseconds (default is 60000ms).

```cpp
// Try to join with a 60 second timeout
bool success = lualtek.join(60000);

if (success) {
  // Join successful, proceed with data transmissions
} else {
  // Join failed (or timed out), handle the error or sleep
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
  // Data send request failed
}
```

### Handling Downlink Messages

To handle downlink messages, you must create a callback function matching the RUI3 signature and register it. Pass the payload to the library instance to handle internal logic (like Duty Cycle updates).

```cpp
void onDownlinkReceived(SERVICE_LORA_RECEIVE_T *payload) {
  // Pass the payload to the library to handle internal commands
  lualtek.onDownlinkReceived(payload);

  // Add your custom downlink logic here
  Serial.printf("Received data on port %d\r\n", payload->Port);
}

// In your setup():
api.lorawan.registerRecvCallback(onDownlinkReceived);
```

#### Supported Downlink Commands

The LualtekRAKRUI library supports the following downlink commands:

- **Change Duty Cycle**: Send a downlink message with `fPort` set to `DOWNLINK_ACTION_CHANGE_INTERVAL_PORT` (3). The payload (first byte) determines the new interval:

  - `0`: 60 minutes
  - `1`: 40 minutes
  - `2`: 30 minutes
  - `3`: 20 minutes
  - `4`: 15 minutes
  - `5`: 10 minutes
  - `6`: 5 minutes
  - `7`: 1 minute
  - `8`: 12 hours
  - `9`: 24 hours

- **Reboot**: Send a downlink message with `fPort` set to `DOWNLINK_ACTION_REJOIN_PORT` (10). No payload is required.

### Scheduling Uplink Transmissions

To schedule uplink transmissions at regular intervals, use the `setupTimers()` method. The library will automatically manage the interval based on the current Duty Cycle configuration.

```cpp
void uplinkRoutine() {
  // Your code to gather sensor data and send it
  lualtek.send(size, data, port);
}

// In setup():
bool success = lualtek.setupTimers(uplinkRoutine);
```

## License

This project is licensed under the [MIT License](LICENSE).
