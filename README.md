# MQTT controlled EPaper for ESP32
ESP32 template to draw on an EPaper display

With the attached code you can control your EPaper Display (like Waveshare) via MQTT commands.

This project uses the following libraries:
- https://github.com/tzapu/WiFiManager
- https://github.com/ZinggJM/GxEPD2
- https://github.com/adafruit/Adafruit_MQTT_Library

| Epaper Pin        | ESP32           | Description  |
| ------------- |:-------------:| -----:|
| VCC   |  3V3 | Power input (3.3V)|
| GND   | GND       |   Ground |
| DIN   | P14       | SPI MOSI pin, data input |
| SCLK  | P13      | SPI CLK pin, clock signal input |
| CS    |  P15       | Chip selection, low active |
| DC    |  P27        | Data/command, low for commands, high for data |
| RST   |  P26        | Reset, low active |
| BUSY  |  P25        | Busy |


Choose display from here
I have Waveshare 2,7 and 4,2 tested and these are in the code

Font

#### MQTT control/syntax
There are 6 different methods you can use:
- drawLine
- fillRect
- drawRect
- printText
- drawAll
- clearScreen

drawLine usage:
drawLine-*from x position*-*from y position*-*to x position*-*to y position*
Example: `drawLine-10-50-10-50`

fillRect & drawRect usage:
fillRect-*from x position*-*from y position*-*width*-*height*
Example: `drawRect-10-50-10-50`

printText usage:
printText-*front name*-*x position*-*y position*-*text to be displayed*
Example: `printText-logisoso50-50-10-Das ist ein Text`

