#!/bin/sh

# Dennis Deng (ddeng@semtech.com)

esp="python /home/dennis/mit/esp-idf/components/esptool_py/esptool/esptool.py"
tty="--port /dev/ttyUSB0 --baud 115200"
args="--chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect"
bootloader="0x1000 /home/dennis/mit/sx1302-esp32-pkt-fwd/build/bootloader/bootloader.bin"
app="0x10000 /home/dennis/mit/sx1302-esp32-pkt-fwd/build/ESXP1302-Pkt-Fwd.bin"
part="0x8000 /home/dennis/mit/sx1302-esp32-pkt-fwd/build/partitions_singleapp.bin"

hd_name=main/global_json.h
json_file=main/conf/global_conf.cn490.json

# prepare the C array comes from global_conf.json
echo 'static uint8_t global_conf[] = {' > $hd_name
scripts/json_to_hex_array.py $json_file >> $hd_name
echo '};' >> $hd_name

make
if [ $? -eq 0 ]; then
	$esp $tty $args $app
	sudo minicom -D /dev/ttyUSB0
fi

