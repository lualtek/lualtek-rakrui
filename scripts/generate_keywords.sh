#!/bin/bash

# This script generates the keywords.txt file for the Arduino IDE
# You should have arduinokeywords installed using pipx

arduino-keywords src -o ..
