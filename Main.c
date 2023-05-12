#include <Wire.h>                       //I2C library
#include <RTClib.h>                     //RTC library
#include "LTC2941.h"                    //Coulomb counter library
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <stdint.h>
#include "TouchScreen.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define Serial SerialUSB
#else
    #define Serial Serial
#endif

#define sensorPin A0

#define YP A2                                                       //Have to be analog pins
#define XM A3
#define TFT_DC 48                                                   //Can be any pin as long as its digital
#define TFT_CS 44
#define YM 42  
#define XP 40

int PWMpin = 4;                                                     //Pin 4 used due to higher base PWM frequency of 980Hz
int blueLED = 5;
int greenLED = 6;
int tempPin = A1;                                                   //TMP36's Analog Vout (Sense) pin is connected to pin A1 on Arduino
int chargerInput = A4;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

float coulomb = 0;
float mAh = 0;
float percent = 0;
float percentTemp = 0;
float prevBatteryTemp = 0;
float currentBatteryTemp = 0;
int dutyCycle;
int dutyCycleTempLoss;                                              //Stores duty cycle decrease due to temperature limit
bool fastCharge = 1;
int8_t chargeLimit = 100;                                           //Sets default maximum battery percentage to 80%
bool alarmEnabled = false;
bool limitButton;
bool speedButton;

RTC_DS1307 rtc;                                                     //Declares RTC object
DateTime currentTime;
uint8_t alarmHour = 7;
uint8_t alarmMinute = 25;
bool isPM = true;
String AMPM;
uint8_t alarmHourDiff;
uint8_t alarmMinuteDiff;

void setup(void) {
    tft.begin();
    loadScreen();                                                   //Turns on screen

    TCCR0B = TCCR0B & B11111000 | B00000010;                        //Sets PWM switching frequency to 7812.5Hz for pins 4/13
    pinMode(PWMpin, OUTPUT);
    pinMode(blueLED, OUTPUT);
    pinMode(greenLED, OUTPUT);

    Wire.begin();
    while (!rtc.begin());                                           //Test for successful connection to DS1307
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                 //Sets RTC to the date & time on the computer when sketch was compiled

    Serial.begin(115200);
    while (!Serial.available());
    ltc2941.initialize();
    ltc2941.setBatteryFullMAh(1000, false);
}

void loop(void) {
    TSPoint p = ts.getPoint();
    if (p.z > ts.pressureThreshhold) {
        if (p.y > 280 && p.y < 400) {                               //Charge speed is being modified
            if (p.x > 210 && p.x < 430 && fastCharge != 0) {
                fastCharge = 0;
                dutyCycle = 167;
                analogWrite(PWMpin, dutyCycle);
                showChargeSpeed();
                delay(10000);
            }
            else if (p.x > 500 && p.x < 830 && fastCharge != 1) {
                fastCharge = 1;
                dutyCycle = 255;
                analogWrite(PWMpin, dutyCycle);
                showChargeSpeed();
                delay(10000);
            }
        }
        else if (p.y > 540 && p.y < 680) {                          //Maximum charge limit is being modified
            if (p.x > 200 && p.x < 390 && chargeLimit > 0) {
                setChargeLimit(-5);
            }
            else if (p.x > 640 && p.x < 840 && chargeLimit < 100) {
                setChargeLimit(5);
            }
        }
        else if (p.y > 690 && p.y < 780 && p.x > 750 && p.x < 830){
            toggleAlarm();
        }
        else if (p.y > 820 && p.y < 890 && p.x > 490 && p.x < 880) {
            setAlarm();
        }
    }
    digitalWrite(greenLED, dutyCycle > 125 && analogRead(chargerInput) > 1000);
    digitalWrite(blueLED, dutyCycle > 230 && analogRead(chargerInput) > 1000);
    

        percent = ltc2941.getPercent();                             //call print/updateBatteryPercentage function - only update if it changes
        Serial.print(percent);
        Serial.print("%, Charge Limit: ");
        Serial.print(chargeLimit);
        Serial.println("%");
        if (percent - percentTemp < 1) {
         if (percent >= chargeLimit) {
             analogWrite(PWMpin, 0);
         }
        percentTemp = percent;
        showBatteryPercentage();
        }

        currentBatteryTemp = getBatteryTemp();                      //Reads current battery temperature
        if (currentBatteryTemp > 45) {
            if (currentBatteryTemp >= prevBatteryTemp) {            //If battery temp exceeds 45°C limit and is not decreasing
                analogWrite(PWMpin, --dutyCycle);
                dutyCycleTempLoss++;
            }
        }
        else {
            if (dutyCycleTempLoss > 0) {                            //If battery temp is under 45°C limit and duty cycle has been temporarily decreased
                analogWrite(PWMpin, ++dutyCycle);
                dutyCycleTempLoss--;
            }
        }
        prevBatteryTemp = currentBatteryTemp;
        delay(500);
}

void loadScreen() {                                                 //Initializes screen upon Arduino startup
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(3);

    uint8_t rotation = 0;                                           //Sets screen orientation to portrait and prints screen
    tft.setRotation(rotation);
    tft.setCursor(0,20);                                            //Prompts user for charge speed preference
    tft.setTextSize(2);
    tft.println("\n\nSelect charge speed");
  
    tft.setTextSize(1);                                             //Prints the charge slider
    tft.setCursor(20, 80);
    tft.print("Slow");
    tft.setCursor(195, 80);
    tft.print("Fast");
    tft.drawRect(20, 90, 200, 30, ILI9341_BLACK);                   //Draws a black rectangle to outline where the slider goes
    showChargeSpeed();

    tft.setCursor(40,145);                                          //Prints maximum charge setting
    tft.setTextSize(2);
    tft.print("Maximum Charge");
    tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setCursor(95, 195);
    tft.print(chargeLimit);
    tft.setCursor(30, 195);
    tft.print("-5");
    tft.setCursor(175, 195);
    tft.print("+5");

    tft.setCursor(15,245);                                          //Prints alarm content
    tft.setTextSize(2);
    tft.println("Alarm Time");
    tft.print(" ");
    tft.setTextSize(3);
    tft.print(alarmHour);
    tft.print(":");
    if (alarmMinute < 9) {
        tft.print("0");
        tft.print(alarmMinute);
        tft.print(" ");
        tft.print(AMPM);
    }
    else {
        tft.print(alarmMinute);
        tft.print(" ");
        tft.print(AMPM);
    }

    tft.fillCircle(200, 255, 10, (alarmEnabled ? ILI9341_GREEN : ILI9341_WHITE));   //Prints alarm toggle
    tft.drawCircle(200, 255, 10, ILI9341_BLACK);
    if (alarmEnabled) {
        calculateTimeDiff();
    }

    tft.setCursor(120, 295);                                        //Prints Set Alarm button
    tft.setTextSize(2);
    tft.print("Set Alarm");
    tft.drawRect(115, 290, 120 , 23, ILI9341_BLACK);

    showBatteryPercentage();
}

void calculateTimeDiff() {
    currentTime = rtc.now();
    alarmHourDiff = ((alarmHour >= currentTime.hour() ? 0 : 24) + alarmHour - currentTime.hour() - (alarmMinute < currentTime.minute() ? 1 : 0) + 24) % 24;
    alarmMinuteDiff = (alarmMinute >= currentTime.minute() ? 0 : 60) + alarmMinute - currentTime.minute();

    Serial.print(alarmHourDiff);
    Serial.print("hr ");
    Serial.print(alarmMinuteDiff);
    Serial.println("min left\n");
    delay(30000);
}

void toggleAlarm(){
    alarmEnabled ^= 1;
    tft.fillCircle(200, 255, 10, (alarmEnabled ? ILI9341_GREEN : ILI9341_WHITE));
    tft.drawCircle(200, 255, 10, ILI9341_BLACK);
    delay(2000);
}

void setAlarm() {
    tft.fillRect(115, 290, 120 , 23, ILI9341_LIGHTGREY);
    tft.setCursor(120, 295);
    tft.setTextSize(2);
    tft.print("Set Alarm");
    delay(300);
    tft.fillRect(115, 290, 120 , 23, ILI9341_WHITE);
    tft.setCursor(120, 295);
    tft.setTextSize(2);
    tft.print("Set Alarm");

    tft.setTextSize(3);                                             //Creates area to set Alarm Hour
    tft.fillRect(0, 145, 240, 175, ILI9341_WHITE);
    tft.setCursor(30,145);
    tft.setTextSize(2);
    tft.print("Set Alarm Hour");
    tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setCursor(110, 195);
    tft.print(alarmHour);
    tft.setCursor(30, 195);
    tft.print("-1");
    tft.setCursor(175, 195);
    tft.print("+1");

    tft.drawRect(20, 260, 100, 28, ILI9341_BLACK);                  //Prints SET button
    tft.setCursor(45, 262);
    tft.setTextSize(3);
    tft.print("SET");
    
    tft.drawRect(130, 260, 100, 28, ILI9341_BLACK);                 //Prints AM/PM toggle button
    tft.setCursor(135, 262);
    tft.setTextSize(3);
    tft.print("AM/PM");
    tft.setCursor(110,290);
    tft.setTextSize(2);
    tft.print("Set to:");
    tft.print(AMPM);

    setAlarmHour();
    setAlarmMinute();
    loadScreen();
    calculateTimeDiff();
}

void setAlarmHour() {
    bool hourSet = false;
    while (hourSet == false) {
        TSPoint p = ts.getPoint();
        if (p.z > ts.pressureThreshhold) {
            if (p.y > 540 && p.y < 680) {
                if (p.x > 190 && p.x < 370) {
                    tft.fillRect(30, 195, 35, 28, ILI9341_LIGHTGREY);
                    tft.setCursor(30, 195);
                    tft.setTextSize(3);
                    tft.print("-1");
                    delay(300);
                    tft.fillRect(30, 195, 35, 28, ILI9341_WHITE);
                    tft.setCursor(30, 195);
                    tft.setTextSize(3);
                    tft.print("-1");
                    tft.fillRect(91, 181, 58, 48, ILI9341_WHITE);   //Don't change this (note to self)
                    alarmHour = (alarmHour + 11) % 12;
                }
                else if (p.x > 640 && p.x < 840) {
                    tft.fillRect(175, 195, 35, 28, ILI9341_LIGHTGREY);
                    tft.setCursor(175, 195);
                    tft.setTextSize(3);
                    tft.print("+1");
                    delay(300);
                    tft.fillRect(175, 195, 35, 28, ILI9341_WHITE);
                    tft.setCursor(175, 195);
                    tft.setTextSize(3);
                    tft.print("+1");
                    tft.fillRect(91, 181, 58, 48, ILI9341_WHITE);   //Don't change this (note to self)
                    alarmHour = (alarmHour + 13) % 12;
                }
                if (alarmHour > 9) {
                    tft.setCursor(105, 195);
                }
                else {
                    tft.setCursor(110, 195);
                }
                tft.print(alarmHour);
            }
            else if (p.y > 740 && p.y < 820) {
                if (p.x > 180 && p.x < 510) {
                    hourSet = true;
                }
                else if (p.x > 540 && p.x < 880) {
                    isPM ^= 1;
                    toggleAMPM();
                }
            }
        }
        delay(1000);
    }
}

void setAlarmMinute() {
    tft.setTextSize(3);
    tft.fillRect(0, 145, 240, 175, ILI9341_WHITE);
    tft.setCursor(30, 145);
    tft.setTextSize(2);
    tft.print("Set Alarm Minute");
    tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setCursor(110, 195);
    tft.print(alarmMinute);
    tft.setCursor(30, 195);
    tft.print("-1");
    tft.setCursor(175, 195);
    tft.print("+1");
    tft.drawRect(70, 260, 100, 28, ILI9341_BLACK);                  //Prints SET button
    tft.setCursor(95, 262);
    tft.setTextSize(3);
    tft.print("SET");
    delay(2000);

    bool minuteSet = false;
    Serial.println(minuteSet);
    while (minuteSet == false) {
        TSPoint p = ts.getPoint();
        if (p.z > ts.pressureThreshhold) {
            if (p.y > 540 && p.y < 680) {
                if (p.x > 190 && p.x < 370) {
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
                    alarmMinute = (alarmMinute + 59) % 60;                    
                }
                else if (p.x > 640 && p.x < 840) {
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
                    alarmMinute = (alarmMinute + 61) % 60; 
                }
                if (alarmMinute > 9) {
                    tft.setCursor(105, 195);
                }
                else {
                    tft.setCursor(110, 195);
                    tft.print("0");
                }
                tft.print(alarmMinute);
            }
            else if (p.y > 750 && p.y < 830 && p.x > 340 && p.x < 680) {
                minuteSet = true;
            }
        }
        delay(1000);
    }
    Serial.println(minuteSet);
    delay(1000);
}

void toggleAMPM() {
    AMPM = isPM ? "PM" : "AM";
    tft.fillRect(0, 290, 240, 30, ILI9341_WHITE);
    tft.setCursor(110, 290);
    tft.setTextSize(2);
    tft.print("Set to:");
    tft.print(AMPM);
    delay(200);
}

float getBatteryTemp() {
    int tempReading = analogRead(tempPin);                          //Reads in and converts TMP36's Vout to int between 0-1023
    float tempVoltage = (tempReading * 5.0) / 1023.0;               //Converts Vout from int to voltage between 0-5.0V
    float tempC = (tempVoltage - 0.5) * 100;                        //Converts voltage to °C with 10mV/°C and 500mV offset
    //Serial.print(tempC);
    //Serial.println("°C");
    return tempC;
}

void showChargeSpeed() {
    if(fastCharge){
        tft.fillRect(20, 90, 100, 30, ILI9341_DARKGREEN);
        tft.fillRect(120, 90, 100, 30, ILI9341_BLUE);
        Serial.println("Fast Charging");
    }
    else {
        tft.fillRect(120, 90, 100, 30, ILI9341_NAVY);
        tft.fillRect(20, 90, 100, 30, ILI9341_GREEN);
        Serial.println("Slow Charging");
    }
    delay(200);
}

void setChargeLimit(int8_t percentChange) {
    if (percentChange > 0) {
        tft.fillRect(175, 195, 35, 28, ILI9341_LIGHTGREY);
        tft.setCursor(175, 195);
        tft.setTextSize(3);
        tft.print("+5");
        delay(2000);
        tft.fillRect(175, 195, 35, 28, ILI9341_WHITE);
        tft.setCursor(175, 195);
        tft.setTextSize(3);
        tft.print("+5");
        tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
    }
    else if (percentChange < 0) {
        tft.fillRect(30, 195, 35, 28, ILI9341_LIGHTGREY);
        tft.setCursor(30, 195);
        tft.setTextSize(3);
        tft.print("-5");
        delay(2000);
        tft.fillRect(30, 195, 35, 28, ILI9341_WHITE);
        tft.setCursor(30, 195);
        tft.setTextSize(3);
        tft.print("-5");
        tft.fillRect(91, 181, 58, 48, ILI9341_WHITE); //Don't change this (note to self)
    }
    chargeLimit += percentChange;
    if (chargeLimit == 100) {
        tft.setCursor(95, 195);
    }
    else {
        tft.setCursor(105, 195);
    }
    tft.print(chargeLimit);
}

void showBatteryPercentage() {
  tft.fillRect(0, 0, 240, 30, ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(165, 15);
  tft.print(percent);
  tft.print("%");
}
