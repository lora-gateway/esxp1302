#!/bin/sh

#
# Note: only run this script after you have already run '. export.sh' under 'esp-idf'.
#
# Dennis Deng (ddeng@semtech.com)
#

pth=`pwd`

esp="$IDF_PYTHON_ENV_PATH/bin/python $IDF_PATH/components/esptool_py/esptool/esptool.py"
tty="--port /dev/ttyUSB0 --baud 921600"
args="--chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect"
bootloader="0x1000 $pth/build/bootloader/bootloader.bin"
app="0x10000 $pth/build/ESXP1302-Pkt-Fwd.bin"
part="0x8000 $pth/build/partitions_singleapp.bin"

hd_name=main/global_json.h
json_file=main/conf/global_conf.cn490.json

if [ "$#" -eq 0 -o "$1" = "-h" -o "$1" = "--help" ]; then
	echo "Usage: $0 [make|flash|flash_all|run]\n"
	exit
fi

if [ "$1" = "make" ]; then
	# prepare the C array comes from global_conf.json
	echo 'static uint8_t global_conf[] = {' > $hd_name
	scripts/json_to_hex_array.py $json_file >> $hd_name
	echo '};' >> $hd_name

	make
fi

if [ "$1" = "flash" ]; then
	$esp $tty $args $app
fi

# choose this when you flash ESP32 for the first time
if [ "$1" = "flash_all" ]; then
	echo $esp $tty $args $bootloader $app $part
fi

if [ "$1" = "run" -o "$2" = "run" ]; then
	sudo minicom -D /dev/ttyUSB0 -con
fi
