# ESXP1302 Gateway User Guide


## Basic Introduction

**ESXP1302 Gateway** project is based on `SX1302\_HAL` project, but uses ESP32 instead of Raspberry Pi. *ESXP1302* = *ESP32* + *SX1302*.

This document is a very simple guide about how to configurate and use ESXP1302 gateway.

First, please be noted that ESXP1302 gateway has 2 types of Wi-Fi working modes, and 2 ways to configurate it.

The 2 Wi-Fi modes are:
- **Soft AP** mode. The gateway works as an Wi-Fi Soft AP, so user can connect to it directly and configurate it.
- **Station** mode. The gateway needs to connect to the same Wi-Fi router as the user and use that router for LoRaWAN uplink and downlink.

The 2 configure ways are:
- **Web UI**. User can change gateway settings by browser after connect to the gateway.
- **Command line** (or Serial Port). There is a simple interactive command line interface available from the USB port, so user can configurate the gateway that way without any network connection required.


## Configurate based on Wi-Fi mode

By default, the gateway boots to the Soft AP mode for the first time. It switches modes when necessary such as Wi-Fi disconnected or be controlled by user. But you can always specify which mode it boots into: just keep one of the Interface board buttons pressed during the poweron: Left button for Soft AP mode, while right button for Station mode.

In the Soft AP mode, user can connect the gateway by Wi-Fi. All the useful information can be seen from the screen, which is: `IP = "192.168.4.1"`; `username = esp32`; `password = esp32wifi`.

Now user can access the web UI by browser with address: `http://192.168.4.1`. Please be noted that the protocol is **http**, not **https**. The default login information are: `username = hello`; `password = world`.

If the gateway boots to Station mode, then both the gateway and user needs to connect to the same wireless router. Then the only difference is user needs to get the right IP address showing on the screen or provided by the router, and access the web UI with that IP address. It **won't** be the "192.168.4.1" any more.

There are also some Wi-Fi mode automatic switch if some events happens:
- In Soft AP mode, if no changes to the web UI and no input to the command line, it reboots to Station mode in 10 minutes.
- If gateway boots to Station mode, but can't connect to wireless router (maybe wrong Wi-Fi settings or haven't set Wi-Fi yet), then it reboots to Soft AP mode in 5 minutes.
- If during the Station mode, `pkt_fwd` works all right for some time, then Wi-Fi disconnected, the gateway will try to re-connect to Wi-Fi for 5 minutes. If succeeds during that time, nothing happens; but if failed, then reboots to Soft AP mode.
- If after power on, the gateway reboots again by `brownout`, then it changes to the other mode. So user should check the screen or the command line output to be sure which mode the gateway is really in.


## Configurate by web UI

From web UI, user can configurate below items:
- **next time boot into**. Generally after the configure is done, the gateway should boot into station mode. But if the gateway will boot somewhere with some unknown Wi-Fi router and needs to provide the SSID informatioin again, then boots into Soft AP mode maybe make more sense. Consider you can always control which mode boots into by the hardware button, this option only becomes important when you can't access the gateway hardware easily.
- **Frequency Plan**. You need to choose this wisely based on your region and node settings. You can click each of them to know the details of the original JSON file used for that frequency plan.
- **Radio 1 and Radio 2 center frequency**. These 2 values should be the reasonable values based on the Frequency plan. Each control the frequency bands. The frequency bands has offsets of -300K/-100K/100K/300K Hz with the center frequency for CN470, while it's -400K/-200K/0K/200K for EU868. So please check the Frequency plan link to be sure.
- **Wi-Fi SSID and password**. Please noted that both only support the ASCII characters. It doesn't work if your SSID or password contains other UTF-8 characters such as Chinese characters. This limitation is hard to remove so you may have to change your Wi-Fi router settings.
- **NS Host and Port**. Here user needs to provide the right NS IP address and port. So far, it seems the gateway doesn't work with domain name, so please please provide IP for the NS Host. This limitation could be removed in the future.
- **Gateway ID**. Each gateway needs to have its own unique ID, so provide it here to avoid using the same one as other gateway.

If you think the original value is correct for any of above field, then you can just leave it empty. Any value you provided would be used to replace the original one from the JSON template file.

You can save your configure whenever you like by clicking the **Apply** button. Such changes are saved in the gateway but won't apply to the current *packet-forward* process yet. It only happens after you click the **Reboot** button to reboot the gateway, or by user reboot the gateway by power off and on.


## Configurate by command line

If you power on the gateway by connecting the micro USB cable to a PC, then you have access to its command line interface. Use any terminal tool such as `sscom` or `putty` to access it with configure as: **115200-8-N-1**.

The command line is the same thing as the serial port, so it outputs gateway logs. When the *packet_forward* is running, the output would mix with any characters input by user. However, if the user ignores that and continues to input or delete characters, the command line will flush itself, so it is still usable. 

When the *packet_forward* doesn't run for reasons such as when Wi-Fi is not connected, then the command line looks like a regular Linux shell interactive window; it even supports basic readline shortcuts such as `Ctrl-A` and `Ctrl-W` etc.

So far, this interface only supports one command named `pkt_fwd`. You can input `pkt_fwd -h` to learn all the options it supports, which, let's be honest, not much. It supports only parts of all the configures available from web UI. Here is the output of `pkt_fwd -h`:
```
 ---- pkt_fwd ----

Available options:
 -h                   print this help
 -u <wifi ssid>       Wifi SSID
 -p <wifi password>   Wifi Password
 --host <NS Host>     NS Host
 --port <NS Port>     NS Port
 --gwid <gateway id>  Gateway ID
```

So, if you need to change other configure items, please use the web UI way.

**Note:**
- Whenever you provide a valid option to `pkt_fwd`, it's saved and the gateway will reboot to apply the new changes.
- To input whitespace in command line (such as for Wi-Fi password, use `\` to escape. For example: `pkt_fwd -u mywifi -p my\ password`.


## Other Useful Documents

For other useful information, please refer to other documents included in the project, include the `README.md`, and a few others located in the folder `doc` including `esxp1302_upgrade_notes.md`, `notes_tips.md` and `todo_issue_list.md`.

