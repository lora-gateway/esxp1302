# About ESXP1302 LoRa Gateway

<p align="center">
  <img src="https://github.com/lora-gateway/esxp1302/raw/main/doc/images/esxp1302-logo.png" alt="ESXP1302 Logo"/>
</p>

ESXP1302 = ESP32 + SX1302.

**ESXP1302** is a low cost 8 channel LoRa gateway design, based on ESP32 platform and porting of
[SX1302\_HAL](https://github.com/Lora-net/sx1302_hal), based on the latest Version *2.1.0*.

<p align="center">
  <img src="https://github.com/lora-gateway/esxp1302/raw/main/doc/images/esxp1302.jpg" alt="ESXP1302 pic"/>
</p>


## How to Compile

There are two ways to compile this project.

### By PlatformIO IDE

[PlatformIO](https://platformio.org/) is a development environment that enables easy multi-platform development and centralized tooling. PlatformIO IDE bases on [Visual Studio Code](https://code.visualstudio.com/).

Follow these steps to compile ESXP1302 and flash the produced firmware.

- Clone this project with: `git clone https://github.com/lora-gateway/esxp1302`.
- Open the newly cloned folder in VS Code. If you do this for the first time, this can take quite some while as PlatformIO will download all the necessary tooling and libraries. Also if platformio is not installed, VS Code will ask you to install it.
- Open your [command palette](https://code.visualstudio.com/docs/getstarted/userinterface#_command-palette) with below shortcuts:
    - Windows/Linux: `Ctrl + Shift + P`
    - Mac: `Command + Shift + P`
- To build the firmware, simply run `PlatformIO: Build` from your command palette.
- To flash the firmware to your device, just run `PlatformIO: Upload` from your command palette.

### By Command line

You need ESP32 development environment [esp-idf](https://github.com/espressif/esp-idf).

Please follow the guide to install the SDK and the tools, or just follow below simplified steps:
```shell
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v4.4.5  # this branch surely works; the latest v5.x branches not yet
git submodule update --init --recursive

# below 2 steps are for Linux. For windows, use `install.bat` and `export.bat` instead.
./install.sh  # install the compilers etc.
. export.sh   # export and set up the environment
```

Then compile this project follow below steps:
```shell
git clone https://github.com/lora-gateway/esxp1302
cd esxp1302
./run_me.sh make  # compile
./run_me.sh flash_all  # flash 3 images: bootloader, partition_table and esxp1302
# ./run_me.sh flash  # use this to only flash esxp1302 firmware
```

Script `run_me.sh` can compile the code, flash the ESP32 board, and open **minicom** to monitor the output, so make sure your ESP32 board is connected.
If you are not under Linux, you need to change `run_me.sh` to use other terminal tool instead or just run your terminal tool manually.

Run `./run_me.sh -h` for its whole usage.


## Configurations

There are 2 compile targets:
- main/packet_forwarder/lora_pkt_fwd.c. This the default target.
- main/libloragw-test/cli4test.c. This provides a command line interface to support several test commands.

It's controlled by flag **CONFIG_LIBLORAGW_TEST**. Change the value to "1" in `run_me.sh` to switch to the second target.


## Usage

At the terminal, you should be able to input the command with your argument of option, such as:
```shell
test_loragw_hal_tx -r 1250 -c 0 -f 505.5 --pwid 22 -b 125 -n 20
test_loragw_hal_tx -r 1250 -c 0 -f 505.5 --pwid 22 -b 125 -n 20 -m CW
test_network_connection -u wireless_ssid -p wireless_password --host 192.168.1.200 --port 2000
pkt_fwd -u wireless_ssid -p wireless_password --host 192.168.1.200 --port 1680
```

Or change settings for `pkt_fwd` from browser:
<p align="center">
  <img src="https://github.com/lora-gateway/esxp1302/raw/main/doc/images/esxp1302-web-settings.png" alt="ESXP1302 Web Settings"/>
</p>

The usage details and all kinds of documents (user guide, changelog, todo list etc.) are available from `doc` folder.

## License

ESXP1302 is distributed under the 3-clause BSD license. See also [License](LICENSE.TXT).
