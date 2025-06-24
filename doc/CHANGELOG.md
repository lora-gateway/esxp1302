# CHANGE LOG


## v0.9.0:
- upgrade to sx1302_hal v2.1.0
- support nodo board
- add option to set NTP server address
- extends us915 TX frequency to cover the range of AS923
- allow up to 1.5MB app size
- remove black web theme
- other fixes to make esxp1302 be more flexible for different hardware designs.


## v0.8.0:
- add hardware reference design materials
- add support for esp32c3
- add support for compiling using PlatformIO
- add support for compiling under Windows (based on PlatformIO)
- optimize memory usage
- update scripts to Python3
- other fixes and document improvement


## v0.7.0:
- show version info in the screen
- change CN470 tx_freq_min from 500MHz to 470MHz for supporting RP2
- fix the bug that ns host can't be domain name
- change web username/password as 'iot/lora'
- add upgrader.py to simplify upgrade the firmware
- add license info


## v0.6.2:
- disable gps output
- workaround to support sx1303


## v0.6.1:
- hide wifi password


## v0.6.0:
- the first public released version
- add version info in the serial port output
- documents
