/**************************************************************
 - DS18B20 temperature sensor 
 - Board: Heltec 8266 WiFi Kit 8 with Display or compatible
 - WiFi enabled:
    -  SSID: Sensor
    - no password
    - IP http://192.168.4.1 (http, not https!)
***************************************************************/

// Import required libraries

// standard Arduino library
#include <Arduino.h>
// one wire library for temperature sensor
#include <OneWire.h>
// library for DS18B20 temperature sensor
#include <DallasTemperature.h>

// OLED display
#include <U8g2lib.h>
// I2C connection of display
#include <Wire.h>

// WiFi
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Pin of the DS18B20 temperature sensor
// setting for generic ESP 8266 boards (if WiFi Kit 8 Board manager is not installed)
// Heltec Wifi Kit 8: D1 = Pin 5
#define ONE_WIRE_BUS 5

// uncomment if Heltec WiFi Kit 8 Board manager is available
// #define ONE_WIRE_BUS D1

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
float tempC = -999.0;      // numerical value
String temperatureC = "";  // formatted string


// Timer variables
unsigned long lastTime = 0;
// read new value every .... ms
unsigned long timerDelay = 1000;

// Initialize Heltec Builtin Display
// reset = 16, inspired by: https://arduino.stackexchange.com/a/95131/98953
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/16);  // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED

// Initialize WiFi
#include "wifi-ap.h" // Webpage template and local utilities
const char* ssid = "Sensor"; // Replace with your network credentials
//const char* password = "pwd"; // no password necessary

// Create AsyncWebServer object on port 80
AsyncWebServer server(80); // http only, not https


void setup() {
  // Serial port for debugging purposes
  Serial.begin(9600);
  Serial.println();

  // Start up the DS18B20 library
  sensors.begin();
  // Send start message to OLED display
  u8g2.begin();
  u8g2.clearBuffer();                  // clear the internal memory
  u8g2.setFont(u8g2_font_crox3hb_tr);  // 12 pixel height
  u8g2.drawStr(4, 14, "T-Sensor");
  u8g2.sendBuffer();
  delay(2000);

  // initialize WiFi AccessPoint
  boolean result = WiFi.softAP(ssid);
  
  // print status to console
  Serial.print("WiFi connection ");
  if(result == false){
    Serial.println("not succesful");
  } else {
    Serial.println("succesful");
  }
  
  // print IP address of the AccessPoint
  Serial.println(WiFi.softAPIP());

  // Route for root-level / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  
  server.begin();

}

void loop() {
  if ((millis() - lastTime) > timerDelay) {

    // Request temperatures from all devices on the bus
    sensors.requestTemperatures();
    tempC = 0.1 * round(10 * sensors.getTempCByIndex(0));

    u8g2.clearBuffer();                 // clear internal display memory
    u8g2.setFont(u8g2_font_luRS18_tf);  // font 18 pixel height

    if (tempC == -127.00) {
      Serial.println("Failed to read from DS18B20 sensor");
      u8g2.drawStr(4, 30, "No Sensor");

    } else {
      Serial.print("Temperature Celsius: ");
      Serial.println(tempC);
      temperatureC = String(tempC, 1);
      u8g2.drawStr(20, 30, temperatureC.c_str());
      u8g2.setFont(u8g2_font_unifont_t_symbols);
      u8g2.drawUTF8(100, 30, "°C");
    }
    u8g2.sendBuffer();
    lastTime = millis();
  }
}
