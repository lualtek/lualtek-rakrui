#!/bin/bash

set -e

# pip3 install clint pyserial setuptools adafruit-nrfutil
# sudo apt-get update

# # make all our directories we need for files and libraries
# mkdir ${HOME}/.arduino15
# mkdir ${HOME}/.arduino15/packages
# mkdir ${HOME}/Arduino
# mkdir ${HOME}/Arduino/libraries

# # install arduino IDE
export PATH=$PATH:$GITHUB_WORKSPACE/bin
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh -s 0.11.0 2>&1
arduino-cli config init > /dev/null
arduino-cli core update-index > /dev/null

## Add arduino links to custom boards
BSP_URLS = "https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless_index.json,https://adafruit.github.io/arduino-board-index/package_adafruit_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://dl.espressif.com/dl/package_esp32_index.json,https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json,https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless.com_rui_index.json"
arduino-cli core update-index --additional-urls $BSP_URLS > /dev/null

## Install RAKWireless boards
arduino-cli core install rak_rui:stm32 > /dev/null
arduino-cli core install rak_rui:nrf52 > /dev/null


