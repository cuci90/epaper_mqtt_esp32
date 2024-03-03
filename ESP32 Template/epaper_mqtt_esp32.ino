/* Wiring 4-line SPI
Pin   ESP32   Description
VCC   3V3   Power input (3.3V)
GND   GND   Ground
DIN   P14   SPI MOSI pin, data input
SCLK  P13   SPI CLK pin, clock signal input
CS    P15   Chip selection, low active
DC    P27   Data/command, low for commands, high for data
RST   P26   Reset, low active
BUSY  P25   Busy status output pin (means busy) 
*/

#include <WiFiManager.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#define USE_HSPI_FOR_EPD
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <U8g2_for_Adafruit_GFX.h> // library for Fonts ----> Choose here https://github.com/olikraus/u8g2/wiki/fntlistall 
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// select the display class (only one), matching the kind of display panel
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW

//#define GxEPD2_DRIVER_CLASS GxEPD2_270_GDEY027T91 // GDEY027T91 176x264, SSD1680, (FB)

#define GxEPD2_DRIVER_CLASS GxEPD2_420     // GDEW042T2   400x300, UC8176 (IL0398), (WFT042CZ15) 

#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x)

#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#elif IS_GxEPD2_7C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))
#endif
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
#endif

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif


// WiFiManager
WiFiManager wifiManager;

// WiFi
char mqtt_server[40] = "yourMQTTBrokerIP"; // MQTT broker address
char mqtt_port[6] = "1883"; // MQTT broker port
char mqtt_username[40] = "youMQTTUserName"; // MQTT username
char mqtt_password[40] = "yourPW"; // MQTT password


// Create an instance of the Adafruit MQTT client
WiFiClient espClient;
Adafruit_MQTT_Client mqtt(&espClient, mqtt_server, atoi(mqtt_port), mqtt_username, mqtt_password);

// Define your MQTT topic
Adafruit_MQTT_Subscribe topic_epaper = Adafruit_MQTT_Subscribe(&mqtt, "epaper/draw");

void setup() {

  // Initialize serial communication
  Serial.begin(115200);


#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
  hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif



  // Initalize Display
  display.init(115200);
  u8g2Fonts.begin(display); // connect u8g2 procedures to Adafruit GFX
  display.setRotation(0); // rotation 1 for 2.7
  uint16_t bg = GxEPD_WHITE;
  uint16_t fg = GxEPD_BLACK;
  u8g2Fonts.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  u8g2Fonts.setFontDirection(0);            // left to right (this is default)
  u8g2Fonts.setForegroundColor(fg);         // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(bg);         // apply Adafruit GFX color
  display.clearScreen();
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(bg);


  // Start WiFiManager
  wifiManager.autoConnect("EpaperAccessPoint"); // Access Point name

  // Initialize MQTT subscription
  mqtt.subscribe(&topic_epaper);


}



void loop() {
  // Ensure the connection to the MQTT server is maintained
  if (!mqtt.connected()) {
    connectToMQTT();
  }

  // Process MQTT messages
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription())) {
    if (subscription == &topic_epaper) {
      // Handle MQTT message
      String message = (char *)topic_epaper.lastread;
      Serial.println("Message MQTT: " + message);
      processMQTTMessage(message);
    }

  }
}

void publishToMQTT(const char* topic, const char* payload) {
  if (mqtt.connected()) {
    Adafruit_MQTT_Publish mqtt_publish(&mqtt, topic , MQTT_QOS_1);
    mqtt_publish.publish(payload);
    Serial.println("MQTT Send");
  } else {
    Serial.println("Not connected to MQTT. Unable to publish.");
  }
}

void connectToMQTT() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT server
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println("Could not connect to MQTT. Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("Connected to MQTT");
}

void processMQTTMessage(String message) {

  String parts[5];
  int separatorIndex = 0;
  int partCount = 0;

  // Split the payload into parts
  for (int i = 0; i < message.length(); i++) {
    if (message.charAt(i) == '~') {
      parts[partCount++] = message.substring(separatorIndex, i);
      separatorIndex = i + 1;
    }

  }

  // Make sure to handle the last part
  if (separatorIndex < message.length()) {
    parts[partCount++] = message.substring(separatorIndex);
  }

  // Check if we have 5 arguments or 1
  if (partCount == 5 || partCount == 1) {
    String methodName = parts[0];
    // part[1] is  x position
    // part[2] is y position
    // parts[3] is width or to position x for drawline
    // parts [4] is height or to position y for drawline

    // For texts:
    // part[1] is front name
    // part[2] is x position
    // parts[3] is y position
    // parts[4] is the text to be displayed

    // Call the appropriate method based on the method name received
    if (methodName.equals("drawLine")) {
      display.drawLine(parts[1].toInt(), parts[2].toInt(), parts[3].toInt(), parts[4].toInt(), GxEPD_BLACK); //x position -- y position to x position -- y position
      Serial.println("Drawing line in buffer");

    } else if (methodName.equals("fillRect")) {
      display.fillRect(parts[1].toInt(), parts[2].toInt(), parts[3].toInt(), parts[4].toInt(), GxEPD_BLACK); //x position -- y position -- width -- height
      Serial.println("Drawing fillRect in buffer");

    } else if (methodName.equals("drawRect")) {
      display.drawRect(parts[1].toInt(), parts[2].toInt(), parts[3].toInt(), parts[4].toInt(), GxEPD_BLACK); //x position -- y position -- width -- height
      Serial.println("Drawing Rect in buffer");

    } else if (methodName.equals("printText")) {
      //printText(param2, param3, param4, param5);
      if (parts[1] == "logisoso50") {
        u8g2Fonts.setFont(u8g2_font_logisoso50_tf);
      } else if (parts[1] == "logisoso22") {
        u8g2Fonts.setFont(u8g2_font_logisoso22_tf);
      } else if (parts[1] == "logisoso16") {
        u8g2Fonts.setFont(u8g2_font_logisoso16_tf);
      } else {
        // default backup font
        u8g2Fonts.setFont(u8g2_font_logisoso16_tf);
      }

      // Convert String to char*
      char text[parts[4].length() + 1];
      parts[4].toCharArray(text, sizeof(text));

      int16_t tw = u8g2Fonts.getUTF8Width(text); // text box width
      int16_t ta = u8g2Fonts.getFontAscent(); 
      int16_t td = u8g2Fonts.getFontDescent(); 
      int16_t th = ta - td; // text box heightv

      //convert arguments to uint16_t to be compatible with u8g2Fonts
      uint16_t x = static_cast<uint16_t>(parts[2].toInt());
      uint16_t y = static_cast<uint16_t>(parts[3].toInt());
      u8g2Fonts.setCursor(x, y); // start writing at this position
      u8g2Fonts.print(text);
      Serial.println("Printing text in buffer");

    } else if (methodName.equals("initialize")) { //initializing again usually not needed as done in setup()
      u8g2Fonts.begin(display); // connect u8g2 procedures to Adafruit GFX
      display.setRotation(1);
      uint16_t bg = GxEPD_WHITE;
      uint16_t fg = GxEPD_BLACK;
      u8g2Fonts.setFontMode(1);                 // use u8g2 transparent mode (this is default)
      u8g2Fonts.setFontDirection(0);            // left to right (this is default)
      u8g2Fonts.setForegroundColor(fg);         // apply Adafruit GFX color
      u8g2Fonts.setBackgroundColor(bg);         // apply Adafruit GFX color
      display.clearScreen();
      display.setFullWindow();
      display.firstPage();
      display.fillScreen(bg);

    } else if (methodName.equals("drawAll")) {
      display.nextPage();
      display.fillScreen(GxEPD_WHITE); //reset all drawings - all is white again
      display.powerOff();
      publishToMQTT("epaper/draw", "ready"); // publish to mqtt that display is ready
      Serial.println("Drawing on display");

    } else if (methodName.equals("clearScreen")) {
      display.clearScreen();
      display.powerOff();
      publishToMQTT("epaper/draw", "ready"); // publish to mqtt that display is ready
      Serial.println("Clearing display");

    }
  }
}
