#!/bin/sh

# Dennis Deng (ddeng@semtech.com)

esp="python /home/dennis/mit/esp-idf/components/esptool_py/esptool/esptool.py"
tty="--port /dev/ttyUSB0 --baud 115200"
args="--chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect"
bootloader="0x1000 /home/dennis/mit/sx1302-esp32-pkt-fwd/build/bootloader/bootloader.bin"
app="0x10000 /home/dennis/mit/sx1302-esp32-pkt-fwd/build/ESXP1302-Pkt-Fwd.bin"
part="0x8000 /home/dennis/mit/sx1302-esp32-pkt-fwd/build/partitions_singleapp.bin"


make
if [ $? -eq 0 ]; then
	$esp $tty $args $app
	sudo minicom -D /dev/ttyUSB0
fi

