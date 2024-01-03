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
part="0x8000 $pth/build/partition_table/partition-table.bin"

web_file=main/webpage.html
web_light_theme_file=main/webpage_light_theme.html
web_hd_name=main/webpage.h

hd_name=main/global_json.h
json_cn_file=main/conf/global_conf.cn490.json
json_eu_file=main/conf/global_conf.eu868.json
json_us_file=main/conf/global_conf.us915.json

if [ "$#" -eq 0 -o "$1" = "-h" -o "$1" = "--help" ]; then
	echo "Usage: $0 [make|make_all|flash|flash_all|run]\n"
	exit
fi

if [ "$1" = "make" ]; then
	# prepare the webpage by dumping it to a string
	scripts/dump_html.py $web_file webpage_str > $web_hd_name
	echo "" >> $web_hd_name  # add a new blank line
	scripts/dump_html.py $web_light_theme_file webpage_light_theme_str >> $web_hd_name

	# prepare the C array comes from global_conf.cn490.json
	echo '// dump from global_conf.cn490.json as string array' > $hd_name
	echo 'static uint8_t global_cn_conf[] = {' >> $hd_name
	scripts/json_to_hex_array.py $json_cn_file >> $hd_name
	echo '};\n' >> $hd_name

	# prepare the C array comes from global_conf.eu868.json
	echo '// dump from global_conf.eu868.json as string array' >> $hd_name
	echo 'static uint8_t global_eu_conf[] = {' >> $hd_name
	scripts/json_to_hex_array.py $json_eu_file >> $hd_name
	echo '};\n' >> $hd_name

	# prepare the C array comes from global_conf.us915.json
	echo '// dump from global_conf.us915.json as string array' >> $hd_name
	echo 'static uint8_t global_us_conf[] = {' >> $hd_name
	scripts/json_to_hex_array.py $json_us_file >> $hd_name
	echo '};\n' >> $hd_name

	# indent the code by prefix 4 ' '.
	sed -i 's/^0x/    0x/; s/ *$//' $hd_name

	idf.py app
fi

if [ "$1" = "make_all" ]; then
	# prepare the C array comes from global_conf.json
	echo 'static uint8_t global_conf[] = {' > $hd_name
	scripts/json_to_hex_array.py $json_file >> $hd_name
	echo '};' >> $hd_name

	idf.py build
fi

if [ "$1" = "flash" ]; then
	$esp $tty $args $app
fi

# choose this when you flash ESP32 for the first time
if [ "$1" = "flash_all" ]; then
	$esp $tty $args $bootloader $app $part
fi

if [ "$1" = "run" -o "$2" = "run" ]; then
	sudo minicom -D /dev/ttyUSB0 -con
fi
