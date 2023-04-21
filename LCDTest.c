#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <stdint.h>
#include "TouchScreen.h"

// For the Adafruit shield, these are the default.
// #define TFT_MISO 50 //Needs to be digital pin 50
// #define TFT_CLK 52 // Needs to be digital pin 52
// #define TFT_RST 49 //Needs to be digital pin 59
// #define TFT_MOSI 51 //Needs to be digital pin 51
#define YP A2 //has to be an analog pin
#define XM A3 //Has to be an analog pin

//Can be any pin as long as its digital
#define TFT_DC 46
#define TFT_CS 48
#define YM 42  
#define XP 40

int ChargeCapacity = 100;


void Startup();
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
TouchScreen ts = TouchScreen(XP,YP,XM,YM,300);

//text size 1 = 7 pixels max
//text size 2 = 14
//text size n = 7*n

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
 
  tft.begin();


  Startup();

}

void loop() {
  
  //this code is only for testing whether i have to update the entire screen or i can only update a certain portion.
  //probably gonna use a method to create a rectangle over the text so i can refresh the screen with a new value/image
  // ;tft.drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
Choose_Charge_Speed();

Choose_Max_Capacity();



}


//Create the menu on inital startup, used in setup function
void Startup(){
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

  //sets screen orientation to portrait
  uint8_t rotation = 0;

  //prints the Title
  tft.setRotation(rotation);
  tft.setCursor(0, 20);
  tft.print("SmartChargeIC");

  //prompts user for charge speed preferenc 
  tft.setTextSize(2);
  tft.println("\n\nPlease select charge speed");
  
  //prints the charge slider
  tft.setTextSize(1);
  tft.setCursor(20, 90);
  tft.print("Slow");
  tft.setCursor(195, 90);
  tft.print("Fast");
  tft.drawRect(20, 100, 200, 30, ILI9341_BLACK); //draws a black rectangle to outline where the slider goes
  tft.fillRect(20,100,100,30,ILI9341_GREEN);
  tft.fillRect(20+100,100,100,30,ILI9341_BLUE);

  //Print Maximum charge setting
  tft.setCursor(40,160);
  tft.setTextSize(2);
  tft.print("Maximum Charge");
  tft.drawRect(90, 195, 60, 50, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(95, 210);
  tft.print(ChargeCapacity);
  tft.setCursor(30, 210);
  tft.print("-5");
  tft.setCursor(175, 210);
  tft.print("+5");

}

void Choose_Charge_Speed(){
  TSPoint p = ts.getPoint();
  if (p.z > ts.pressureThreshhold && p.y >315 && p.y<420) {
    if(p.x>210 && p.x<430){
        tft.fillRect(20,100,100,30,ILI9341_DARKGREEN);
        tft.fillRect(20+100,100,100,30,ILI9341_BLUE);
        Serial.print("Slow Charging\n");
        delay(200);
    }
    if(p.x>550 && p.x<820){
      tft.fillRect(20+100,100,100,30,ILI9341_NAVY);
      tft.fillRect(20,100,100,30,ILI9341_GREEN);
      Serial.print("Fast Charging\n");
      delay(200);
    }
  }
}

void Choose_Max_Capacity(){
  TSPoint p = ts.getPoint();
  if(p.z>ts.pressureThreshhold && p.y >615 && p.y<715){
    if(p.x >230 && p.x < 280 && ChargeCapacity > 0 ){
        tft.fillRect(30, 210, 35, 28, ILI9341_LIGHTGREY);
        tft.setCursor(30, 210);
        tft.setTextSize(3);
        tft.print("-5");
        delay(300);
        tft.fillRect(30, 210, 35, 28, ILI9341_WHITE);
        tft.setCursor(30, 210);
        tft.setTextSize(3);
        tft.print("-5");
        tft.fillRect(91, 196, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
        tft.setCursor(105, 210);
        ChargeCapacity -= 5;
        tft.print(ChargeCapacity);

    }
    if(p.x>745 && p.x<805 && ChargeCapacity < 100){
        tft.fillRect(175, 210, 35, 28, ILI9341_LIGHTGREY);
        tft.setCursor(175, 210);
        tft.setTextSize(3);
        tft.print("+5");
        delay(300);
        tft.fillRect(175, 210, 35, 28, ILI9341_WHITE);
        tft.setCursor(175, 210);
        tft.setTextSize(3);
        tft.print("+5");
        tft.fillRect(91, 196, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
        ChargeCapacity += 5;
        if(ChargeCapacity == 100){
        tft.setCursor(95, 210);
        tft.print(ChargeCapacity);
        }
        else{
        tft.setCursor(105, 210);
        tft.print(ChargeCapacity);
        }
    }
  }
}
