# SNES2USBDual
Adapter for two Super Nintendo (SNES) gamepads using an Arduino Leonardo Pro Micro.

## What you need:
* Arduino Pro Micro (Leonardo)
* 2 SNES female conectors - you can cut an extension cable.
* USB-C cable.
* Soldering skills.

## How to:
* Solder cables as schematic image.
* Install HID-Project library in Arduino IDE.
* Upload sketch to your board.
* Play.

## Diagram:
![Diagrama](./snes-usb-adapter-wiring.png)

## SNES Controller Pinout

 -----------------\\\
| 1 2 3 4 | 5 6 7  |\
 -----------------\/\
Pin 1: 5V\
Pin 2: Data_Clock (Arduino: Pin 6)\
Pin 3: Data_Latch (Arduino: Pin 7)\
Pin 4: Data_Serial (Arduino: Pin 12)\
Pin 5: N/A\
Pin 6: N/A\
Pin 7: GND

