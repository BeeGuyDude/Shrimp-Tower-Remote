#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266HTTPClient.h>

//OLED Configuration
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SDA 2   //ESP-01S Wiring
#define SCL 0   //ESP-01S Wiring

//Wifi/Server Configuration
const char* ssid = "Shrimpternet Beacon";
const char* password = "pimpshrimpin";
const char* timeAddress = "http://192.168.4.1/time";
const char* overrideAddress = "http://192.168.4.1/override";
const char* daylightPeriodAddress = "http://192.168.4.1/daylight-period";

//Devices
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  //Configure I2C for the fuckass wiring of the ESP-01S and start OLED
  Wire.begin(2, 0); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //OLED Initialization
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Shrimpternet End-"));
  display.println(F("point Initializing..."));
  display.display();
  delay(3000);

  //Wifi initialization
  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting to");\
  display.println("Shrimpternet");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Shrimpternet Found!");
  display.print(F("IP Address: "));
  display.println(WiFi.localIP());
  display.display();
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Initialized.");
  display.display();
}

void loop() {
  //Clear display and add basic readout
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(" -SHRIMPTERNET DATA-");

  //Do HTTP GET requests and format accordingly
  WiFiClient client;
  HTTPClient http;

  http.begin(client, timeAddress);
  http.GET();
  display.print("Time: ");
  display.println(http.getString());
  http.end();

  http.begin(client, daylightPeriodAddress);
  http.GET();
  display.print("Period: ");
  display.println(http.getString());
  http.end();

  http.begin(client, overrideAddress);
  http.GET();
  display.print("Override: ");
  display.println(http.getString());
  http.end();

  display.display();
  delay(3000);
}


