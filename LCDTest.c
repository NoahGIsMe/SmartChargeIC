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
#define TFT_DC 48
#define TFT_CS 44
#define YM 42  
#define XP 40

int ChargeCapacity = 100;
int AlarmHour = 1;
int AlarmMinute = 0;
float percentage;
bool alarmToggle = false;
bool isPM = false;
String AMPM = "AM";
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
TouchScreen ts = TouchScreen(XP,YP,XM,YM,300);

void Startup();
void Choose_Charge_Speed();
void Choose_Max_Capacity();
void Set_Alarm();
void Set_Alarm_Hour();
void Set_Alarm_Minute();
void Toggle_Alarm();
void Toggle_AM_PM();
void batteryPercentage();

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
Choose_Charge_Speed();

Choose_Max_Capacity();

Set_Alarm();

Toggle_Alarm();

batteryPercentage();
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

  //prompts user for charge speed preferenc 
  tft.setCursor(0,20);
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
  tft.print(" ");
  tft.setTextSize(3);
  tft.print(AlarmHour);
  tft.print(":");
  if(AlarmMinute < 9){
    tft.print("0");
    tft.print(AlarmMinute);
    tft.print(" ");
    tft.print(AMPM);
  }
  else{
    tft.print(AlarmMinute);
    tft.print(" ");
    tft.print(AMPM);
  }
  //Prints Alarm Toggle
  if(alarmToggle == false){
    tft.fillCircle(200, 255, 10, ILI9341_WHITE);
    tft.drawCircle(200, 255, 10, ILI9341_BLACK);
  }
  else if(alarmToggle == true){
    tft.fillCircle(200, 255, 10, ILI9341_GREEN);
    tft.drawCircle(200,255,10, ILI9341_BLACK);
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
        tft.drawRect(20, 260, 100, 28, ILI9341_BLACK);
        tft.setCursor(45, 262);
        tft.setTextSize(3);
        tft.print("SET");
        Toggle_AM_PM();
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

      else if(p.x>745 && p.x<805 && AlarmHour < 23){
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
        if(p.x>160 && p.x<500){
        alarmSet = true;
        }
      }

      Toggle_AM_PM();

  }while(alarmSet == false);

  // Startup();
  Set_Alarm_Minute();
}

void Set_Alarm_Minute(){
        tft.setTextSize(3);
        tft.fillRect(0, 145, 240, 175, ILI9341_WHITE);
        tft.setCursor(30,145);
        tft.setTextSize(2);
        tft.print("Set Alarm Minute");
        tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
        tft.setTextSize(3);
        tft.setCursor(110, 195);
        tft.print(AlarmMinute);
        tft.setCursor(30, 195);
        tft.print("-1");
        tft.setCursor(175, 195);
        tft.print("+1");

        //Prints SET button
        tft.drawRect(70, 260, 100, 28, ILI9341_BLACK);
        tft.setCursor(95, 262);
        tft.setTextSize(3);
        tft.print("SET");

    bool alarmSet;

  do{

    alarmSet = false;
    TSPoint p = ts.getPoint();
    
    if(p.z>ts.pressureThreshhold && p.y >580 && p.y<680){

      if(p.x >230 && p.x < 280 && AlarmMinute > 0 ){
          tft.fillRect(30, 195, 35, 28, ILI9341_LIGHTGREY);
          tft.setCursor(30, 195);
          tft.setTextSize(3);
          tft.print("-1");
          delay(100);
          tft.fillRect(30, 195, 35, 28, ILI9341_WHITE);
          tft.setCursor(30, 195);
          tft.setTextSize(3);
          tft.print("-1");
          tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
          AlarmMinute -= 1;
          if(AlarmMinute > 9){
          tft.setCursor(105, 195);
          tft.print(AlarmMinute);
          }

          else{
          tft.setCursor(110, 195);
          tft.print(AlarmMinute);
          }
      }

      else if(p.x>745 && p.x<805 && AlarmMinute < 59){
          tft.fillRect(175, 195, 35, 28, ILI9341_LIGHTGREY);
          tft.setCursor(175, 195);
          tft.setTextSize(3);
          tft.print("+1");
          delay(100);
          tft.fillRect(175, 195, 35, 28, ILI9341_WHITE);
          tft.setCursor(175, 195);
          tft.setTextSize(3);
          tft.print("+1");
          
          tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
          AlarmMinute += 1;
          if(AlarmMinute > 9){
          tft.setCursor(105, 195);
          tft.print(AlarmMinute);
          }

          else{
          tft.setCursor(110, 195);
          tft.print("0");
          tft.print(AlarmMinute);
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
 
}

void Toggle_Alarm(){
  
  TSPoint p = ts.getPoint();

  if(p.z> ts.pressureThreshhold && p.y > 700 && p.y < 770){
    if(p.x > 750 && p.x < 830){
      if(alarmToggle == true){
      alarmToggle = false;
      tft.fillCircle(200, 255, 10, ILI9341_WHITE);
      tft.drawCircle(200, 255, 10, ILI9341_BLACK);
      delay(200);
      }
      else if(alarmToggle == false){
      alarmToggle = true;
      tft.fillCircle(200, 255, 10, ILI9341_GREEN);
      tft.drawCircle(200, 255, 10, ILI9341_BLACK);
      delay(200);
      }
    }
  }
}

void Toggle_AM_PM(){
  //Print isAM toggle button
  tft.drawRect(130, 260, 100, 28, ILI9341_BLACK);
  tft.setCursor(135, 262);
  tft.setTextSize(3);
  tft.print("AM/PM");
  tft.setCursor(110,290);
  tft.setTextSize(2);
  tft.print("Set to:");
  tft.print(AMPM);

  TSPoint p = ts.getPoint();

  if(p.z> ts.pressureThreshhold && p.y > 735 && p.y < 830){
    if(p.x > 590 && p.x < 890){
      if(isPM == true){
      isPM = false;
      AMPM = "AM";
      tft.fillRect(0, 290, 240, 30, ILI9341_WHITE);
      tft.setCursor(110,290);
      tft.print("Set to:");
      tft.print(AMPM);
      delay(200);
      }
      else if(isPM == false){
      isPM = true;
      AMPM = "PM";
      tft.fillRect(0, 290, 240, 30, ILI9341_WHITE);
      tft.setCursor(110,290);
      tft.print("Set to:");
      tft.print(AMPM);
      delay(200);
      }
    }
  }
}

void batteryPercentage(){
  tft.fillRect(0, 0, 240, 30, ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(165, 15);
  tft.print(percentage);
  tft.print("%");
}
