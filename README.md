# About ESXP1302 Packet Forwarder

This is the ESP32 platform porting of SX1302\_HAL project (https://github.com/Lora-net/sx1302\_hal), based on the Version 1.0.5.
I try to stick to and not change the original code, but removed some line-seperating comments I think not necessary.
As for ESP32 part, I do it the way whatever I think the best, which also imports some differences.


## Upgrade and Usage guide

The documents about how to upgrade the released binaries, and how to use this gateway are both located under the `doc` folder.
Below is mainly for development and other utilities.


## How to Compile

First, you need ESP32 development environment "esp-idf". You can get it from https://github.com/espressif/esp-idf.
Please follow the guide to install the SDK as well as the tools.

Below are the simplified steps for Linux platform:
```shell
mkdir ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
# git checkout v4.0  # optional; this is the status I used for early development
# git checkout 4a011f31  # optional; this is the status after I upgraded the esp-idf with commit '8b9050a' at 2021-12-03.
./install.sh  # install the compilers etc.
. export.sh   # export and set up the environment
```

Then you can compile this project by first change to the project folder, and use `./run_me.sh` to build and flash it.
Run `./run_me.sh -h` for its usage.

This script can compile the code, flash the ESP32 board, and open **minicom** to monitor the output, so make sure your ESP32 board is connected.
If you are not under Linux, you need to change `run_me.sh` to use other terminal tool instead or just run your terminal tool manually.


## Configurations

There are some compile targets indeed, just as under the original `sx1302_hal` project under `libloragw/tst/test_*`.
But you have to choose only one for compilation by change the file **main/main.c**, which is a symbol link to one of `test/test_*` files.
The default is `test/lora_pkt_fwd.c`, but you can change to others such as `test/test_loragw_hal_rx.c` before you run `run_me.sh`.


## Usage

At the terminal, you should be able to input the command with your argument of option, such as:
```shell
test_loragw_hal_tx -r 1250 -c 0 -f 505.5 --pwid 22 -b 125 -n 20
test_loragw_hal_tx -r 1250 -c 0 -f 505.5 --pwid 22 -b 125 -n 20 -m CW
test_network_connection -u wireless_ssid -p wireless_password --host 192.168.1.200 --port 2000
pkt_fwd -u wireless_ssid -p wireless_password --host 192.168.1.200 --port 1680
```


## Issues and TODOs

There are some issues need to be fixed, and more features under development.

Below is the list of some of them:
1. A few test targets do no work: `test_loragw_cal.c`, `test_loragw_counter.c`, `test_loragw_capture_ram.c` and `test_loragw_gps.c`.
2. GPS can output raw data with the module **ATGM336H** but hasn't been fully verified yet.
3. Needs to upgrade to sx1302\_hal v2.1.0 to support SF5 and SF6.
