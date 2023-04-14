#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 46
#define TFT_CS 48
#define TFT_MISO 50
#define TFT_CLK 52
#define TFT_RST 49
#define TFT_MOSI 51



// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//text size 1 = 7 pixels max
//text size 2 = 14
//text size n = 7*n

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
 
  tft.begin();

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

  // Serial.print(F("Text                     "));
  // Serial.println(testText());
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

  //sets screen orientation to portrait
  uint8_t rotation = 0;

  //prints the Title
  tft.setRotation(rotation);
  tft.setCursor(0, 20);
  tft.print("SmartChargeIC");

  //prompts user for charge speed preference
  tft.setTextSize(2);
  tft.println("\n\nPlease select charge speed");
  
  //prints the charge slider
  tft.setTextSize(1);
  tft.setCursor(20, 90);
  tft.print("Slow");
  tft.setCursor(195, 90);
  tft.print("Fast");
  tft.drawRect(20, 100, 200, 30, ILI9341_BLACK); //draws a black rectangle to outline where the 
  tft.fillRect(20,100,100,30,ILI9341_GREEN);
  tft.fillRect(20+100,100,100,30,ILI9341_BLUE);

  //Print Maximum charge setting
  tft.setCursor(0,175);
  
  tft.print("Maximum Charge");

  // tft.fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
}

void loop() {
  // put your main code here, to run repeatedly:
  tft.setCursor(195,90);
  tft.fillRect(0, 240, 320, 10, ILI9341_BLACK);
  tft.fillRect(0, 240, 320, 10, ILI9341_BLUE);
  tft.fillRect(0, 240, 320, 10, ILI9341_WHITE);
  // ;tft.drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);


}
