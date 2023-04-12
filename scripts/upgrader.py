#!/usr/bin/python3

# Upgrader for upgrading released ESXP1302 images
# Run 'pyinstaller --onefile upgrader.py' to produce frozen binary
#
# License: Revised BSD License, see LICENSE.TXT file including in the project
#
# Dennis Deng (ddeng@semtech.com)
# 2022.07.15
#

import os
import sys
import serial
import serial.tools.list_ports
import subprocess


def get_port_name():
    brand_list = ['USB']
    ports = serial.tools.list_ports.comports()
    ports = [str(p) for p in ports]
    for p in ports:
        for brand_str in brand_list:
            if p.find(brand_str) >= 0:
                return p.split()[0]

    # no port found
    print("Error! No port found which belongs to list %s!" %brand_list)
    print("Only below ports available:")
    for p in ports: print(p)
    sys.exit()


cmd_str = 'esptool.py --baud 921600 --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect'

if len(sys.argv) != 2 and len(sys.argv) != 4 and len(sys.argv) != 6:
    print("Usage:\n\tupgrader ESXP1302-Pkt-Fwd.bin")
    print("Or:\tupgrader ESXP1302-Pkt-Fwd.bin -p COM5")
    print("Or:\tupgrader ESXP1302-Pkt-Fwd.bin bootloader.bin partition-table.bin")
    print("Or:\tupgrader ESXP1302-Pkt-Fwd.bin bootloader.bin partition-table.bin -p COM5")
    sys.exit()

com = ''
for i, x in enumerate(sys.argv):
    if x == '-p':
        com = sys.argv[i+1]
        sys.argv.remove(sys.argv[i+1])
        sys.argv.remove(sys.argv[i])
        break

if com == '':
    com = get_port_name()


# check files exist or not
if not os.path.exists(sys.argv[1]):
    print("file '%s' doesn't exist. Check and try again" %sys.argv[1])
    sys.exit()
if len(sys.argv) == 4:
    if not os.path.exists(sys.argv[2]):
        print("file '%s' doesn't exist. Check and try again" %sys.argv[2])
        sys.exit()
    if not os.path.exists(sys.argv[3]):
        print("file '%s' doesn't exist. Check and try again" %sys.argv[3])
        sys.exit()


print("\nNote: you may need to press either esp32 button before or during the upgrade process\n")
#input("Press 'Enter' to start...")

cmd_list = cmd_str.split()
cmd_list[1:1] = ['--port', com]  # put it in the same position
cmd_list += ['0x10000', sys.argv[1]]

if len(sys.argv) == 4:
    if sys.argv[2] == 'partition-table.bin' and sys.argv[3] == 'bootloader.bin':
        cmd_list += ['0x8000', sys.argv[2]]
        cmd_list += ['0x1000', sys.argv[3]]
    else:  # bootloader, partable
        cmd_list += ['0x1000', sys.argv[2]]
        cmd_list += ['0x8000', sys.argv[3]]

print('cmd:', ' '.join(cmd_list))
subprocess.call(cmd_list)
#subprocess.call(cmd_list, shell=True)  # this is for windows
