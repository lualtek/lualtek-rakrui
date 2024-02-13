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
import glob
import os
import subprocess
import collections

# Global Variables
BUILD_DIR = ""
CROSS = "\N{cross mark}"
CHECK = "\N{check mark}"
SUCCESS = False
LIBRARY_NAME = ""
EXAMPLES_FOLDER = ""

ALL_PLATFORMS = {
    # classic Arduino AVR
    "uno": "arduino:avr:uno",
    "leonardo": "arduino:avr:leonardo",
    "mega2560": "arduino:avr:mega:cpu=atmega2560",
    # Arduino SAMD
    "zero": "arduino:samd:arduino_zero_native",
    "cpx": "arduino:samd:adafruit_circuitplayground_m0",
    # Espressif
    "esp8266": "esp8266:esp8266:huzzah:eesz=4M3M,xtal=80",
    "esp32": "esp32:esp32:featheresp32:FlashFreq=80",
    "rak4631": "rakwireless:nrf52:WisCoreRAK4631Board:softdevice=s140v6,debug=l0",
    "rak4631-rui": "rak_rui:nrf52:WisCoreRAK4631Board:softdevice=s140v6,debug=l0",
    "rak3172-evaluation-rui": "rak_rui:stm32:WisDuoRAK3172EvaluationBoard,debug=l0",
    "rak3172-T-rui": "rak_rui:stm32:WisDuoRAK3172TBoard,debug=l0",
    "rak11200": "rakwireless:esp32:WisCore_RAK11200_Board",
    "rak11300": "rakwireless:mbed_rp2040:WisCoreRAK11300Board",
    # groupings
    "rak_platforms": ("rak4631", "rak11200", "rak11300"),
    "rak_platforms-test": ("rak4631", "rak11200", "rak11300"),
    "rak_platforms_rui-test": (
        "rak4631-rui",
        "rak3172-evaluation-rui",
        "rak3172-T-rui",
    ),
}

BSP_URLS = "https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless_index.json,https://adafruit.github.io/arduino-board-index/package_adafruit_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://dl.espressif.com/dl/package_esp32_index.json,https://sandeepmistry.github.io/arduino-nRF5/package_nRF5_boards_index.json,https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless.com_rui_index.json"


class ColorPrint:
    @staticmethod
    def print_fail(message, end="\n"):
        sys.stdout.write("\x1b[1;31m" + message.strip() + "\x1b[0m" + end)

    @staticmethod
    def print_pass(message, end="\n"):
        sys.stdout.write("\x1b[1;32m" + message.strip() + "\x1b[0m" + end)

    @staticmethod
    def print_warn(message, end="\n"):
        sys.stdout.write("\x1b[1;33m" + message.strip() + "\x1b[0m" + end)

    @staticmethod
    def print_info(message, end="\n"):
        sys.stdout.write("\x1b[1;34m" + message.strip() + "\x1b[0m" + end)

    @staticmethod
    def print_bold(message, end="\n"):
        sys.stdout.write("\x1b[1;37m" + message.strip() + "\x1b[0m" + end)


def install_platform(platform):
    print("Installing", platform, end=" ")
    if platform == "adafruit:avr":
        install_platform("arduino:avr")
    if (
        os.system(
            "arduino-cli core install "
            + platform
            + " --additional-urls "
            + BSP_URLS
            + " > /dev/null"
        )
        != 0
    ):
        ColorPrint.print_fail("FAILED to install " + platform)
        exit(-1)
    ColorPrint.print_pass(CHECK)


def run_or_die(cmd, error):
    print(cmd)
    if os.system(cmd) != 0:
        ColorPrint.print_fail(error)
        exit(-1)


def get_library_name():
    try:
        libprop = open(BUILD_DIR + "/library.properties")
        for line in libprop:
            if line.startswith("name="):
                return line.replace("name=", "").strip()
    except OSError:
        pass
    return None


def install_dependencies():
    try:
        libprop = open(BUILD_DIR + "/library.properties")
        for line in libprop:
            if line.startswith("depends="):
                deps = line.replace("depends=", "").split(",")
                for dep in deps:
                    dep = dep.strip()
                    print("Installing " + dep)
                    run_or_die(
                        'arduino-cli lib install "' + dep + '" > /dev/null',
                        "FAILED to install dependency " + dep,
                    )
    except OSError:
        print("No library dep or properties found!")
        pass


def test_examples_in_folder(fqbn):
    # Get all the folders in the example folders with a .ino file
    folders_in_examples_folder = [
        EXAMPLES_FOLDER + "/" + example
        for example in os.listdir(EXAMPLES_FOLDER)
        if os.path.isdir(EXAMPLES_FOLDER + "/" + example)
        and os.path.exists(EXAMPLES_FOLDER + "/" + example + "/" + example + ".ino")
    ]

    # Make library available inside the example folder, copy src/* into example folder with the name inside library.properties
    run_or_die(
        f"cp -r {BUILD_DIR}/src/* {EXAMPLES_FOLDER}/{LIBRARY_NAME}",
        "FAILED to copy library to example folder",
    )

    for single_example_folder in folders_in_examples_folder:
        print("\t" + single_example_folder, end=" ")

        examplepath = (
            single_example_folder
            + "/"
            + os.path.basename(single_example_folder)
            + ".ino"
        )
        cmd = ["arduino-cli", "compile", "--fqbn", fqbn, examplepath]
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        r = proc.wait()
        out = proc.stdout.read()
        err = proc.stderr.read()
        if r == 0:
            ColorPrint.print_pass(CHECK)
        else:
            ColorPrint.print_fail(CROSS)
            ColorPrint.print_fail(out.decode("utf-8"))
            ColorPrint.print_fail(err.decode("utf-8"))
            SUCCESS = True


def setup_arduino_cli():
    print()
    ColorPrint.print_info("#" * 40)
    print("INSTALLING ARDUINO BOARDS")
    ColorPrint.print_info("#" * 40)

    run_or_die(
        "arduino-cli core update-index --additional-urls " + BSP_URLS + " > /dev/null",
        "FAILED to update core indices",
    )

    print()
    ColorPrint.print_info("#" * 40)
    print("INSTALLING DEPENDENCIES")
    ColorPrint.print_info("#" * 40)
    install_dependencies()
    ColorPrint.print_info(
        "Libraries installed: ", glob.glob(os.environ["HOME"] + "/Arduino/libraries/*")
    )


def main():
    LIBRARY_NAME = get_library_name()
    ColorPrint.print_info("Library name:", LIBRARY_NAME)

    try:
        BUILD_DIR = os.environ["GITHUB_WORKSPACE"]
    except KeyError:
        pass  # If not using GitHub Actions

    os.environ["PATH"] += os.pathsep + BUILD_DIR + "/bin"
    ColorPrint.print_info("Build dir:", BUILD_DIR)
    EXAMPLES_FOLDER = os.path.join(BUILD_DIR, "examples")
    ColorPrint.print_info("Examples folder:", EXAMPLES_FOLDER)

    setup_arduino_cli()

    # Test platforms
    platforms = []

    for arg in sys.argv[1:]:
        platform = ALL_PLATFORMS.get(arg, None)
        if isinstance(platform, str):
            platforms.append(arg)
        elif isinstance(platform, collections.abc.Iterable):
            for p in platform:
                platforms.append(p)
        else:
            print("Unknown platform: ", arg)
            exit(-1)

    for platform in platforms:
        fqbn = ALL_PLATFORMS[platform]
        print("#" * 80)
        ColorPrint.print_info("SWITCHING TO " + fqbn)
        # Take only first two elements of fqbn, as the third one is the board name
        install_platform(":".join(fqbn.split(":", 2)[0:2]))
        print("#" * 80)
        test_examples_in_folder(fqbn)

    exit(SUCCESS)


if __name__ == "__main__":
    main()
