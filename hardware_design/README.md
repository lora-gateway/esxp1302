# Hardware Design

The hardware of ESXP1302 mainly includes 3 parts:
- SX1302 based ref design
- Motherboard connecting SX1302 and ESP32 and screen etc.
- Outside case


## SX1302 based ref design

For this part, we just re-use the ref design provided by Semtech.

It provided in this address:
    https://www.semtech.com/products/wireless-rf/lora-core/sx1302cssxxxgw1
Under "Reference Designs" --> "USB Corecell ref design for LBT Spectral Scan gateway production package for China"

The 7z file is downloaded from that place and put under "sx1302/" folder for convenient.

**Note**:
- This ref design is for CN470 frequency band. In order to support other regions, users need to modify it accordingly.
- Since esxp1302 only uses the SPI interface for communication and the USB part is not used, so such part shouldn't be soldered (which also can save some BOM cost).


## Motherboard

This is a very simple interface board used to connect other components. Besides of the SX1302 board and ESP32 board, it contains a few other parts:
- **An screen**. It is optional and used to display some helpful information (such as Wi-Fi status; NS info; time etc.).
- **A GPS chip**. This is also optional; It can provide PPS pulse which could be useful for accurate time sync, maybe can be useful for Class B (but not implemented yet).
- **2 Buttons**. They purpose of them is for choosing Wi-Fi mode when boot, and for other potential usage not implemented yet.

The design material is under "motherboard/" folder.


## Outside case

The outside case include 2 parts:
- transparent top layer
- solid white botton layer

Both parts can be 3D printed. The design material is under "case/" folder.
