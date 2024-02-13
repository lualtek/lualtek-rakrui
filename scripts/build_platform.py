"""
This script is designed to test Arduino platform compatibility for different boards.

Usage:
    1. Ensure that the necessary environment variables are set or adjust the script as needed.
    2. Run the script with the desired platform(s) as command-line arguments.
       Example: python build_platform.py uno esp8266
    3. The script will install the required Arduino boards, dependencies, and test example sketches.
    4. Any failures during the testing process will be reported.

Examples:
    - Test specific platforms:
        python build_platform.py uno esp8266
    - Test a group of platforms (e.g., main_platforms, arcada_platforms):
        python build_platform.py main_platforms arcada_platforms
    - Test all platforms defined in the script:
        python build_platform.py ${!rak_platforms-test}

"""

import sys
import subprocess
from pathlib import Path
import os

# Constants
CROSS_MARK = "\N{cross mark}"
CHECK_MARK = "\N{check mark}"
BSP_URLS = (
    "https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless_index.json,"
    "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json,"
    "http://arduino.esp8266.com/stable/package_esp8266com_index.json,"
    "https://dl.espressif.com/dl/package_esp32_index.json,"
    "https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json,"
    "https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless.com_rui_index.json"
)

ALL_PLATFORMS = {
    "uno": "arduino:avr:uno",
    "leonardo": "arduino:avr:leonardo",
    "mega2560": "arduino:avr:mega:cpu=atmega2560",
    "zero": "arduino:samd:arduino_zero_native",
    "cpx": "arduino:samd:adafruit_circuitplayground_m0",
    "esp8266": "esp8266:esp8266:huzzah:eesz=4M3M,xtal=80",
    "esp32": "esp32:esp32:featheresp32:FlashFreq=80",
    "rak4631": "rakwireless:nrf52:WisCoreRAK4631Board:softdevice=s140v6,debug=l0",
    "rak4631-rui": "rak_rui:nrf52:WisCoreRAK4631Board:softdevice=s140v6,debug=l0",
    "rak3172-evaluation-rui": "rak_rui:stm32:WisDuoRAK3172EvaluationBoard:debug=l0",
    "rak3172-T-rui": "rak_rui:stm32:WisDuoRAK3172TBoard:debug=l0",
    "rak11200": "rakwireless:esp32:WisCore_RAK11200_Board",
    "rak11300": "rakwireless:mbed_rp2040:WisCoreRAK11300Board",
    "rak_platforms": ("rak4631", "rak11200", "rak11300"),
    "rak_platforms-test": ("rak4631", "rak11200", "rak11300"),
    "rak_platforms_rui-test": (
        "rak4631-rui",
        "rak3172-evaluation-rui",
        "rak3172-T-rui",
    ),
}


class ColorPrint:
    @staticmethod
    def print_fail(message, end="\n"):
        print(f"\x1b[1;31m{message.strip()}\x1b[0m", end=end)

    @staticmethod
    def print_pass(message, end="\n"):
        print(f"\x1b[1;32m{message.strip()}\x1b[0m", end=end)


def run_command(command, fail_message):
    """Run a shell command with subprocess.run and handle failure."""
    result = subprocess.run(
        command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True
    )
    if result.returncode != 0:
        ColorPrint.print_fail(f"{fail_message}\n{result.stderr}")
        sys.exit(-1)
    return result


def install_platform(fqbn):
    """Install the specified platform using arduino-cli."""
    print(f"Installing {fqbn} ", end="")
    command = f"arduino-cli core install {fqbn} --additional-urls {BSP_URLS}"
    run_command(command, f"FAILED to install {fqbn}")
    ColorPrint.print_pass(CHECK_MARK)


def test_examples_in_folder(examples_folder, fqbn):
    """Test all examples in the specified folder for the given FQBN."""
    success = True
    for example in examples_folder.glob("*/"):
        if (example / f"{example.name}.ino").exists():
            print(f"\tTesting {example.name} ", end="")
            command = f"arduino-cli compile --fqbn {fqbn} {example}"
            result = subprocess.run(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                shell=True,
            )
            if result.returncode == 0:
                ColorPrint.print_pass(CHECK_MARK)
            else:
                ColorPrint.print_fail(CROSS_MARK)
                ColorPrint.print_fail(result.stdout + "\n" + result.stderr)
                success = False
    return success


def main():
    build_dir = Path(os.getenv("GITHUB_WORKSPACE", Path.cwd()))
    examples_folder = build_dir / "examples"

    # Flatten the platform list in case of groups
    platforms_to_test = []
    for arg in sys.argv[1:]:
        if arg in ALL_PLATFORMS:
            platform = ALL_PLATFORMS[arg]
            if isinstance(platform, tuple):
                platforms_to_test.extend(platform)
            else:
                platforms_to_test.append(arg)
        else:
            print(f"Unknown platform or group: {arg}")
            sys.exit(-1)

    setup_commands = [
        f"arduino-cli core update-index --additional-urls {BSP_URLS}",
    ]
    for command in setup_commands:
        run_command(command, "Setup command failed")

    overall_success = True
    for platform_key in platforms_to_test:
        fqbn = ALL_PLATFORMS[platform_key]
        print(f"Testing platform: {platform_key} with FQBN {fqbn}")
        install_platform(
            fqbn.split(":")[0:2]
        )  # Install platform without board specifics
        success = test_examples_in_folder(examples_folder, fqbn)
        overall_success &= success

    sys.exit(0 if overall_success else -1)


if __name__ == "__main__":
    main()
