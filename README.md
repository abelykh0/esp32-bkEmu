# esp32-bkEmu
BK-0010 Emulator

## Hardware
[VGA32 v1.4 Board](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1083)

## What it can do
* Emulate [BK-0010](https://en.wikipedia.org/wiki/Electronika_BK)

## Third party software
This project uses several libraries:
* (GPL v3.0) Display video using VGA, process PS/2 keyboard, and sound output: https://github.com/fdivitto/FabGL
* (unsure, MIT?) PDP-11 emulator by Eric A. Edwards: https://github.com/emestee/bk-emulator

## Plans for the future / issues
* Load file in .BIN format from SD card
* Save snapshot
* Measure emulation speed and see if it needs a slowdown
* Sound
