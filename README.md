# MQTT controlled EPaper for ESP32
ESP32 template to draw "freely" on an EPaper display via MQTT

With the attached code you can control your EPaper Display (like Waveshare) via MQTT commands.

This project uses the following libraries:
- https://github.com/tzapu/WiFiManager
- https://github.com/ZinggJM/GxEPD2
- https://github.com/adafruit/Adafruit_MQTT_Library

### Wiring

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

### WiFi
After first flash the ESP will open a AccesPoint called *EpaperAccessPoint*. Connect to this WiFi with your phone and set up the connection to your network.

### MQTT Setup
Replace the following constants in the code by your respective MQTT Broker configuration

`char mqtt_server[40] = "yourMQTTBrokerIP"; // MQTT broker address`

`char mqtt_port[6] = "1883"; // MQTT broker port`

`char mqtt_username[40] = "youMQTTUserName"; // MQTT username`

`char mqtt_password[40] = "yourPW"; // MQTT password`


### Choose a display
Check GxEPD2 Library to see if your display is supported. Copy your display driver class definition over to the template.
I have tested Waveshare 2.7 and 4.2 and both display definitions are in the code.

Example: `#define GxEPD2_DRIVER_CLASS GxEPD2_420`

### Chosse font
You can easily add different fonts. Look for a u8g2 font.
Then add in the code an additonal else-if part with the name of the font.

Example:

`else if (parts[1] == "logisoso22") {
        u8g2Fonts.setFont(u8g2_font_logisoso22_tf);
      }`



### MQTT control/syntax
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
fillRect-from x position-*from y position*-*width*-*height*

Example: `drawRect-10-50-10-50`

printText usage:
printText-*front name*-*x position*-*y position*-*text to be displayed*

Example: `printText-logisoso50-50-10-Das ist ein Text`

