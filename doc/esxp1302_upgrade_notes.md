# Notes for ESXP1302 Gateway Upgrade


## firmware upgrade preparation

You'll need `esptool` to upgrade firmware for ESXP1302.
Install it with:
```shell
pip install esptool
```

If failed, try `python -m pip install esptool` or `pip3 install esptool`.

You may need `setuptools` first. You can install it with `pip install setuptools`.

Now you can check if it's ready by run `esptool.py` or `python -m esptool`.


## upgrade firmware

There are two ways to upgrade the firmware.

### Fully upgrade

You'll need to use this upgrade way if the ESP32 is never been upgraded.

Put the firmware file(s) somewhere, then run below command under that same foler for fully upgrade:
```shell
esptool.py --port /dev/ttyUSB0 --baud 921600 --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 ESXP1302-Pkt-Fwd.bin 0x1000 bootloader.bin 0x8000 partition-table.bin
```

You may need to change the port name(`/dev/ttyUSB0`) to the right value, for example, it would be `COMx` under Windows.

### Application upgrade

The way only upgrade the Application part, mainly used to replace an older version.

The command becomes:
```shell
esptool.py --port /dev/ttyUSB0 --baud 921600 --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 ESXP1302-Pkt-Fwd.bin
```

**Note:** If you got error as `Failed to connect to ESP32: Timed out waiting for packet header` or `Wrong boot mode detected`, then just try the upgrade command again while pressing one of the esp32's small buttons during the upgrade command running.

