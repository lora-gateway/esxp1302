#!/usr/bin/python3

# Upgrader for upgrading released ESXP32 images
#
# Dennis Deng (ddeng@semtech.com)
# 2022.07.15
#

import os
import sys
import subprocess

cmd_str = 'esptool.py --port /dev/ttyUSB0 --baud 921600 --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect'

if len(sys.argv) != 2 and len(sys.argv) != 4:
    print("Usage:\n\tupgrader ESXP1302-Pkt-Fwd.bin")
    print("Or:\tupgrader ESXP1302-Pkt-Fwd.bin bootloader.bin partition-table.bin")
    sys.exit()

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


print("Note: you may need to press either esp32 button before or during the upgrade process")
input("Enter any key to start...")

cmd_list = cmd_str.split()
cmd_list += ['0x10000', sys.argv[1]]

if len(sys.argv) == 4:
    if sys.argv[2] == 'partition-table.bin' and sys.argv[3] == 'bootloader.bin':
        cmd_list += ['0x8000', sys.argv[2]]
        cmd_list += ['0x1000', sys.argv[3]]
    else:  # bootloader, partable
        cmd_list += ['0x1000', sys.argv[2]]
        cmd_list += ['0x8000', sys.argv[3]]

subprocess.call(cmd_list)
