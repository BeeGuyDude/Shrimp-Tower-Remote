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
#define BUTTON_PIN 1 //Shared with TX pin but no issue

//Wifi/Server Configuration
const char* ssid = "Shrimpternet Beacon";
const char* password = "pimpshrimpin";
const char* timeAddress = "http://192.168.4.1/time";
const char* daylightPeriodAddress = "http://192.168.4.1/daylight-period";
const char* overrideGetAddress = "http://192.168.4.1/override";
const char* overrideSetAddress = "http://192.168.4.1/override-set";

String timeString = "";
String periodString = "";
String overrideString = "";

unsigned long previousMillis = 0;
const int httpUpdateDelayMillis = 3000;

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
  display.println(F("Shrimpternet Tap"));
  display.println(F("Initializing..."));
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
  delay(500);
}

void loop() {
  //Store time for manual periodic http updates
  unsigned long currentMillis = millis();
  
  //Clear display and reset cursor
  display.clearDisplay();
  display.setCursor(0,0);

  //Do HTTP GET requests and format accordingly
  WiFiClient client;
  HTTPClient http;

  //Check override button state, active high
  bool overrideButtonPressed = digitalRead(BUTTON_PIN);

  if (!overrideButtonPressed) {
    display.setTextSize(1);
    display.println(" -SHRIMPTERNET DATA-");

    //Only update data periodically so as to not spam server with updates
    //TODO: Add rollover protection to this
    if (currentMillis - previousMillis >= httpUpdateDelayMillis) {
      //HTTP client is dumb, needs to be reinitialized for address each time rather than using different endpoints for same address
      //Time update
      http.begin(client, timeAddress);
      http.GET();
      timeString = http.getString();
      http.end();
      //Daylight period update
      http.begin(client, daylightPeriodAddress);
      http.GET();
      periodString = http.getString();
      http.end();
      //Override update
      http.begin(client, overrideGetAddress);
      http.GET();
      overrideString = http.getString();
      http.end();

      //Update display only when new information is received
      display.print("Time: ");
      display.println(timeString);
      display.print("Period: ");
      display.println(periodString);
      display.print("Override: ");
      display.println(overrideString);
      display.display();

      previousMillis = currentMillis;
    }

    //Brief cycle delay
    delay(100);
  } else {  //Button was pressed, override requested
    //HTTP GET request procs the light for override transition on its end
    http.begin(client, overrideSetAddress);
    http.GET();
    overrideString = http.getString();
    http.end();

    display.setTextSize(2);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); //Draw black text on lit background
    display.print("!OVERRIDE!");
    display.display();
    delay(200);
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); //Draw lit text on black background
    display.display();
    delay(200);

    previousMillis = currentMillis - httpUpdateDelayMillis;
  }
}


