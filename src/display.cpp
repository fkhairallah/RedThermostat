/*
 * ********************************************************************************

   Routines to manage display

   bitmap can be created from BMP files using @ http://javl.github.io/image2cpp/

   AdaFruit SSD1306 library documentation:
    https://cdn-learn.adafruit.com/downloads/pdf/adafruit-gfx-graphics-library.pdf

 * ********************************************************************************
*/
#include <RedGlobals.h>

#ifdef DISPLAY_PRESENT
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold24pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 'wifi', 9x6px
// converted using http://javl.github.io/image2cpp/
#define WIFI_HEIGHT   6
#define WIFI_WIDTH    9
const unsigned char wifiBitmap [] PROGMEM = {
  0x00, 0x80, 0x02, 0x80, 0x0a, 0x80, 0x2a, 0x80, 0xaa, 0x80, 0xaa, 0x80
};

// 'heat-icon', 20x9px
#define HEAT_HEIGHT   9
#define HEAT_WIDTH    20
const unsigned char heatBitmap [] PROGMEM = {
  0x11, 0x11, 0x10, 0x22, 0x22, 0x20, 0x44, 0x44, 0x40, 0x22, 0x22, 0x20, 0x11, 0x11, 0x10, 0x22,
  0x22, 0x20, 0x44, 0x44, 0x40, 0x22, 0x22, 0x20, 0x11, 0x11, 0x10
};


bool displayPresent = false;
bool displayDimState;
Ticker displayDimmerTicker;

bool displayOn;
long displayMillis;         // dim the display at this timestamp



void showHeat()
{
#ifdef BUTTONS_PRESENT
  if (heatIsOn)
  {
    display.drawBitmap(0 , display.height()-HEAT_HEIGHT, heatBitmap, HEAT_WIDTH, HEAT_HEIGHT, 1);
  }
#endif
}

void showWiFi()
{

  if (WiFi.status() == WL_CONNECTED)
  {

    display.drawBitmap(display.width()  - WIFI_WIDTH , WIFI_HEIGHT,
                       wifiBitmap, WIFI_WIDTH, WIFI_HEIGHT, 1);
  }
}


void blink_the_display()
{
  display.dim(displayDimState);
  displayDimState = !displayDimState;
}

void displayBlink(bool blink)
{
  if (!displayPresent) return;

  if (blink)
  {
    displayDimmerTicker.attach(1, blink_the_display);
  }
  else
  {
    displayDimmerTicker.detach();
  }

}




bool configureDisplay()
{
  Wire.begin(SDA_pin, SCL_pin);  // Setup I2C library

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    console.println(F("SSD1306 allocation failed"));
  }
  else
  {
    display.clearDisplay(); // Clear the buffer
    display.setTextColor(WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    displayPresent = true;  // and signal we are ready
    console.println("I2C Display configured");
  }
  return displayPresent;
}

// This routine will service dimming the display then turning it off after a preset period
// of time. This is necessary in dark room and at night
void serviceDisplay()
{
  if (displayOn)
  {
    if (millis() > (displayMillis + (unsigned long)(_DISPLAY_INTERVAL * 3)))
    {
      display.clearDisplay();
      display.display();
      displayOn = false;
    }
    else if (millis() > (displayMillis + (unsigned long)_DISPLAY_INTERVAL))
    {
      display.dim(true);
    }
  }
}

void displayStatus()
{
  if (!displayPresent) return;

  display.clearDisplay();
  display.dim(false);
  display.setFont();
  display.setCursor(0, 0);
  display.println("[RED]Thermostat " + String(VERSION));
  display.println(WiFi.localIP().toString());
  display.print("Location: ");
  display.println(deviceLocation);
  display.print("MQTT: ");
  display.println(mqttServer);
  display.print("Found " + String(numberOfDevices));
  display.println(" sensors");
  display.println("long press = Status;");
  display.display();
  displayOn = true;
  displayMillis = millis();

}

void displayTemperature(float temp)
{
  int16_t x1, y1;
  uint16_t w, h;
  char tempStr[16];

  if (!displayPresent) return;

  display.clearDisplay();
  display.dim(false);
  display.setTextSize(0);

  showWiFi();
  showHeat();

  display.setFont(&FreeSansBold24pt7b);

  if (temp > -900)
    sprintf(tempStr, "%.1f", temp);
  else
    sprintf(tempStr, "NR");
  display.getTextBounds(tempStr, 0, 50, &x1, &y1, &w, &h);
  display.setCursor((display.width() - w) / 2, (display.height() + h) / 2); // Start at top-left corner
  display.print(tempStr);
  display.display();
  displayOn = true;
  displayMillis = millis();


}

#ifdef BUTTONS_PRESENT
void displayRequiredTemperature(int temp)
{

  int16_t x1, y1;
  uint16_t w, h;
  char tempStr[16];

  if (!displayPresent) return;

  display.clearDisplay();
  display.dim(false);
  display.setTextSize(0);
  showWiFi();
  showHeat();

  display.setFont(&FreeSansBold24pt7b);
  sprintf(tempStr, "%i", temp);
  display.getTextBounds(tempStr, 0, 50, &x1, &y1, &w, &h);
  display.setCursor((display.width() - w) / 2, (display.height() + h) / 2); // Start at top-left corner
  display.print(tempStr);

  display.setFont();
  display.setCursor(0, 0);
  char lineText[16];
  sprintf( lineText, "SET%c", 0x10);
  display.println(lineText);
 
  display.display();
  displayOn = true;
  displayMillis = millis();
}
#endif

#endif