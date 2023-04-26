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

// #define interruptPin 2

int ChargeCapacity = 100;
int AlarmHour = 1;
// int AlarmHour2 = 1;
int AlarmMinute = 1;
bool alarmToggle = false;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
TouchScreen ts = TouchScreen(XP,YP,XM,YM,300);

void Startup();
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC


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

Set_Alarm();

// Toggle_Alarm();

// keyPad();
// delay(3000);




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
  tft.println("\n\nSelect charge speed");
  
  //prints the charge slider
  tft.setTextSize(1);
  tft.setCursor(20, 80);
  tft.print("Slow");
  tft.setCursor(195, 80);
  tft.print("Fast");
  tft.drawRect(20, 90, 200, 30, ILI9341_BLACK); //draws a black rectangle to outline where the slider goes
  tft.fillRect(20,90,100,30,ILI9341_GREEN);
  tft.fillRect(120,90,100,30,ILI9341_BLUE);

  //Print Maximum charge setting
  tft.setCursor(40,145);
  tft.setTextSize(2);
  tft.print("Maximum Charge");
  tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(95, 195);
  tft.print(ChargeCapacity);
  tft.setCursor(30, 195);
  tft.print("-5");
  tft.setCursor(175, 195);
  tft.print("+5");

  //Prints Alarm Stuffs
  tft.setCursor(15,245);
  tft.setTextSize(2);
  tft.println("Alarm Time");
  tft.print("   ");
  tft.setTextSize(3);
  tft.print(AlarmHour);
  tft.print(":");
  tft.print(AlarmMinute);
  
  //Prints Alarm Toggle
  if(alarmToggle == false){
    tft.fillCircle(200, 255, 10, ILI9341_WHITE);
    tft.drawCircle(200, 255, 10, ILI9341_BLACK);
  }
  else if(alarmToggle == true){
    tft.fillCircle(200, 255, 10, ILI9341_GREEN);
  }

  //Prints Set Alarm Button
  tft.setCursor(120, 295);
  tft.setTextSize(2);
  tft.print("Set Alarm");
  tft.drawRect(115, 290, 120 , 23, ILI9341_BLACK);

}

void Choose_Charge_Speed(){
  TSPoint p = ts.getPoint();
  if (p.z > ts.pressureThreshhold && p.y >295 && p.y<400) {
    if(p.x>210 && p.x<430){
        tft.fillRect(20,90,100,30,ILI9341_DARKGREEN);
        tft.fillRect(20+100,90,100,30,ILI9341_BLUE);
        Serial.print("Slow Charging\n");
        delay(200);
    }
    else if(p.x>550 && p.x<820){
      tft.fillRect(20+100,90,100,30,ILI9341_NAVY);
      tft.fillRect(20,90,100,30,ILI9341_GREEN);
      Serial.print("Fast Charging\n");
      delay(200);
    }
  }
}

void Choose_Max_Capacity(){
  TSPoint p = ts.getPoint();
  if(p.z>ts.pressureThreshhold && p.y >580 && p.y<680){

    if(p.x >230 && p.x < 280 && ChargeCapacity > 0 ){
        tft.fillRect(30, 195, 35, 28, ILI9341_LIGHTGREY);
        tft.setCursor(30, 195);
        tft.setTextSize(3);
        tft.print("-5");
        delay(300);
        tft.fillRect(30, 195, 35, 28, ILI9341_WHITE);
        tft.setCursor(30, 195);
        tft.setTextSize(3);
        tft.print("-5");
        tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
        tft.setCursor(105, 195);
        ChargeCapacity -= 5;
        tft.print(ChargeCapacity);
    }

    else if(p.x>745 && p.x<805 && ChargeCapacity < 100){
        tft.fillRect(175, 195, 35, 28, ILI9341_LIGHTGREY);
        tft.setCursor(175, 195);
        tft.setTextSize(3);
        tft.print("+5");
        delay(300);
        tft.fillRect(175, 195, 35, 28, ILI9341_WHITE);
        tft.setCursor(175, 195);
        tft.setTextSize(3);
        tft.print("+5");
        
        tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
        ChargeCapacity += 5;
        if(ChargeCapacity == 100){
        tft.setCursor(95, 195);
        tft.print(ChargeCapacity);
        }

        else{
        tft.setCursor(105, 195);
        tft.print(ChargeCapacity);
        }

    }

  }

}


void Set_Alarm(){
  TSPoint p = ts.getPoint();
  if(p.z>ts.pressureThreshhold && p.y >835 && p.y<880){
    if(p.x >535 && p.x < 830){
        tft.fillRect(115, 290, 120 , 23, ILI9341_LIGHTGREY);
        tft.setCursor(120, 295);
        tft.setTextSize(2);
        tft.print("Set Alarm");
        tft.fillRect(115, 290, 120 , 23, ILI9341_WHITE);
        tft.setCursor(120, 295);
        tft.setTextSize(2);
        tft.print("Set Alarm");

        //Creates area to set Alarm Hour
        tft.setTextSize(3);
        tft.fillRect(0, 145, 240, 175, ILI9341_WHITE);
        tft.setCursor(30,145);
        tft.setTextSize(2);
        tft.print("Set Alarm Hour");
        tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
        tft.setTextSize(3);
        tft.setCursor(110, 195);
        tft.print(AlarmHour);
        tft.setCursor(30, 195);
        tft.print("-1");
        tft.setCursor(175, 195);
        tft.print("+1");

        //Prints SET button
        tft.drawRect(70, 260, 100, 28, ILI9341_BLACK);
        tft.setCursor(95, 262);
        tft.setTextSize(3);
        tft.print("SET");

        Set_Alarm_Hour();
    }
  }

}

void Set_Alarm_Hour(){
  bool alarmSet;

  do{

    alarmSet = false;
    TSPoint p = ts.getPoint();
      
    if(p.z>ts.pressureThreshhold && p.y >580 && p.y<680){

      if(p.x >230 && p.x < 280 && AlarmHour > 0 ){
          tft.fillRect(30, 195, 35, 28, ILI9341_LIGHTGREY);
          tft.setCursor(30, 195);
          tft.setTextSize(3);
          tft.print("-1");
          delay(300);
          tft.fillRect(30, 195, 35, 28, ILI9341_WHITE);
          tft.setCursor(30, 195);
          tft.setTextSize(3);
          tft.print("-1");
          tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
          AlarmHour -= 1;
          if(AlarmHour > 9){
          tft.setCursor(105, 195);
          tft.print(AlarmHour);
          }

          else{
          tft.setCursor(110, 195);
          tft.print(AlarmHour);
          }
      }

      else if(p.x>745 && p.x<805 && AlarmHour < 24){
          tft.fillRect(175, 195, 35, 28, ILI9341_LIGHTGREY);
          tft.setCursor(175, 195);
          tft.setTextSize(3);
          tft.print("+1");
          delay(300);
          tft.fillRect(175, 195, 35, 28, ILI9341_WHITE);
          tft.setCursor(175, 195);
          tft.setTextSize(3);
          tft.print("+1");
          
          tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
          AlarmHour += 1;
          if(AlarmHour > 9){
          tft.setCursor(105, 195);
          tft.print(AlarmHour);
          }

          else{
          tft.setCursor(110, 195);
          tft.print(AlarmHour);
          }

      }

  }

      if(p.y>760 && p.y<830){
        if(p.x>350 && p.x<620){
        alarmSet = true;
        }
      }


  }while(alarmSet == false);

  Startup();
  // Set_Alarm_Minute();



}


// void Set_Alarm_Minute(){



  
// }

// void Toggle_Alarm(){

// }
