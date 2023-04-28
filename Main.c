#include <Wire.h>                       //I2C library
#include <EEPROM.h>                     //EEPROM editor library
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

#define aref_voltage 3.3                                            //ARef pin is tied to 3.3V to decrease noise in TMP36 temperature reading
#define sensorPin A0

#define YP A2                                                       //Have to be analog pins
#define XM A3
#define TFT_DC 48                                                   //Can be any pin as long as its digital
#define TFT_CS 44
#define YM 42  
#define XP 40

int PWMpin = 4;                                                     //Pin 4 used due to higher base PWM frequency of 980Hz
int tempPin = A1;                                                   //TMP36's Analog Vout (Sense) pin is connected to pin A1 on Arduino
int blueLED = 5;

float coulomb = 0;
float mAh = 0;
float percent = 0;
float prevBatteryTemp = 0;
float currentBatteryTemp = 0;
int dutyCycle;
int dutyCycleTempLoss;                                              //Stores duty cycle decrease due to temperature limit
int interruptCounter;
bool fastCharge;
int8_t chargeLimit = 100;                                           //Sets default maximum battery percentage to 80%
bool alarmEnabled;
bool limitButton;
bool speedButton;

RTC_DS1307 rtc;                                                     //Declares RTC object
DateTime currentTime;
uint8_t alarmHour;
uint8_t alarmMinute;
bool isPM;
String AMPM = "AM";
uint8_t alarmHourDiff;
uint8_t alarmMinuteDiff;

//post-testing table of duty cycle values vs. battery percentage/voltage (3-7-4.2V)

void setup(void) {
    TCCR0B = TCCR0B & B11111000 | B00000010;                        //Sets PWM switching frequency to 7812.5Hz for pins 4/13
    pinMode(PWMpin, OUTPUT);
    analogReference(EXTERNAL);                                      //Need to set aref_voltage to 3.3V
    noInterrupts();                                                 //Temporarily disables all interrupts
    TCCR1A = 0;                                                     //Initializes 16-bit Timer1
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A = 62500;                                                  //Sets Output Compare Register (16MHz clock/256 prescaler/1Hz timer)
    TCCR1B |= (1 << WGM12);                                         //CTC mode clears timer when timer counter reaches OCR register value
    TCCR1B |= (1 << CS12);                                          //Sets prescaler to 256
    TIMSK1 |= (1 << OCIE1A);                                        //Enables timer compare interrupt
    interrupts();                                                   //Enables all interrupts

    Wire.begin();
    while (!rtc.begin());                                           //Test for successful connection to DS1307
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                 //Sets RTC to the date & time on the computer when sketch was compiled

    Serial.begin(115200);
    while (!Serial.available());
    ltc2941.initialize();
    ltc2941.setBatteryFullMAh(1000);
    
    tft.begin();
    loadScreen();                                                   //Turns on screen
}

void loop(void) {
    TSPoint p = ts.getPoint();
    if (p.z > ts.pressureThreshhold) {
        if (p.y > 295 && p.y < 400) {                               //Charge speed is being modified
            if (p.x > 210 && p.x < 430 && fastCharge != 0) {
                fastCharge = 0;
                EEPROM.update(0, fastCharge);
                showChargeSpeed();
            }
            else if (p.x > 550 && p.x < 820 && fastCharge != 1) {
                fastCharge = 1;
                EEPROM.update(0, fastCharge);
                showChargeSpeed();
            }
        }
        else if (p.y > 580 && p.y < 680) {                          //Maximum charge limit is being modified
            if (p.x > 230 && p.x < 280 && chargeLimit > 0) {
                setChargeLimit(-5);
            }
            else if (p.x > 745 && p.x < 805 && chargeLimit < 100) {
                setChargeLimit(5);
            }
        }
        else if (p.y > 700 && p.y < 770 && p.x > 750 && p.x < 830){
            toggleAlarm();
        }
        else if (p.y > 835 && p.y < 880 && p.x > 535 && p.x < 830) {
            setAlarm();
        }
    }
    
    //coulomb = ltc2941.getCoulombs();                                //reads charge in coulombs
    //mAh = ltc2941.getmAh();                                         //reads charge in mAh
    //percent = ltc2941.getPercent();                                 //reads battery percentage
    //setAlarm();                                                     //set the alarm
    //getBatteryTemp(void);                                           //get the battery temperature
    //setChargeSpeed();                                               //set the charge speed
    //setMaxChargeLimit();                                            //set the charge capacity
}

ISR(TIMER1_COMPA_vect) {                                            //Timer1 compare interrupt service routine
    if (++interruptCounter %= 5) {
        percent = ltc2941.getPercent();                             //call print/updateBatteryPercentage function - only update if it changes

        currentBatteryTemp = getBatteryTemp()                       //Reads current battery temperature
        if (currentBatteryTemp > 45) {
            if (currentBatteryTemp >= prevBatteryTemp) {            //If battery temp exceeds 45°C limit and is not decreasing
                //analogWrite(PWMpin, dutyCycle[getBatteryPercent] - ++dutyCycleTempLoss);   //Decrease PWM duty cycle/charging current and record that duty cycle has been temporarily decreased
            }
        }
        else {
            if (dutyCycleTempLoss > 0) {                            //If battery temp is under 45°C limit and duty cycle has been temporarily decreased
                //analogWrite(PWMpin, dutyCycle[getBatteryPercent] - --dutyCycleTempLoss);   //Increase duty cycle until it is reaches normal values
            }
        }
        prevBatteryTemp = currentBatteryTemp;
    }
}

void loadScreen() {                                                 //Initializes screen upon Arduino startup
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(3);

    showBatteryPercentage();

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
    tft.fillRect(20,90,100,30,ILI9341_GREEN);
    tft.fillRect(120,90,100,30,ILI9341_BLUE);

    tft.setCursor(40,145);                                          //Prints maximum charge setting
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
}

void calculateTimeDiff() {
    currentTime = rtc.now();
    alarmHourDiff = ((alarmHour >= currentTime.hour() ? 0 : 24) + alarmHour - currentTime.hour() - (alarmMinute < currentTime.minute() ? 1 : 0) + 24) % 24;
    alarmMinuteDiff = (alarmMinute >= currentTime.minute() ? 0 : 60) + alarmMinute - currentTime.minute();
}

void toggleAlarm(){
    alarmEnabled ^= 1;
    EEPROM.update(2, alarmEnabled);
    tft.fillCircle(200, 255, 10, (alarmEnable ? ILI9341_GREEN : ILI9341_WHITE));
    tft.drawCircle(200, 255, 10, ILI9341_BLACK);
    delay(200);
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
}

void setAlarmHour() {
    bool alarmSet = false;
    while (alarmSet == false) {
        TSPoint p = ts.getPoint();
        if (p.z > ts.pressureThreshhold) {
            if (p.y > 580 && p.y < 680) {
                if (p.x > 230 && p.x < 280) {
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
                else if (p.x > 745 && p.x < 805) {
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
            else if (p.y > 735 && p.y < 830) {
                if (p.x > 160 && p.x < 500) {
                    alarmSet = true;
                }
                else if (p.x > 590 && p.x < 890) {
                    toggleAMPM();
                }
            }
        }
    }
    //Startup();
    EEPROM.update(3, alarmHour + isPM * 12);
    setAlarmMinute();
}

void setAlarmMinute() {
    tft.setTextSize(3);
    tft.fillRect(0, 145, 240, 175, ILI9341_WHITE);
    tft.setCursor(30, 145);
    tft.setTextSize(2);
    tft.print("Set Alarm Minute");
    // tft.drawRect(90, 180, 60, 50, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setCursor(110, 195);
    tft.print(alarmMinute);
    // tft.setCursor(30, 195);
    // tft.print("-1");
    // tft.setCursor(175, 195);
    // tft.print("+1");
    tft.drawRect(70, 260, 100, 28, ILI9341_BLACK);                  //Prints SET button
    tft.setCursor(95, 262);
    tft.setTextSize(3);
    tft.print("SET");

    bool alarmSet = false;
    while (alarmSet = false) {
        TSPoint p = ts.getPoint();
        if (p.z > ts.pressureThreshhold) {
            if (p.y > 580 && p.y < 680) {
                if (p.x > 230 && p.x < 280) {
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
                else if (p.x > 745 && p.x < 805) {
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
            else if (p.y > 735 && p.y < 830 && p.x > 350 && p.x < 620) {
                alarmSet = true;
            }
        }
    }

    EEPROM.update(4, alarmMinute);
    loadScreen();
}

void toggleAMPM() {
    AMPM = isPM ? "PM" : "AM";
    tft.fillRect(0, 290, 240, 30, ILI9341_WHITE);
    tft.setCursor(110,290);
    tft.print("Set to:");
    tft.print(AMPM);
    delay(200);
}

float getBatteryTemp() {
    int tempReading = analogRead(tempPin);                          //Reads in and converts TMP36's Vout to int between 0-1023
    float tempVoltage = (tempReading * aref_voltage) / 1023.0;      //Converts Vout from int to voltage between 0-3.3V
    float tempC = (tempVoltage - 0.5) * 100;                        //Converts voltage to °C with 10mV/°C and 500mV offset
    Serial.print(tempC);
    Serial.println("°C");
    return tempC;
}

void showChargeSpeed() {
    if(fastCharge){
        tft.fillRect(20,90,100,30,ILI9341_DARKGREEN);
        tft.fillRect(120,90,100,30,ILI9341_BLUE);
        Serial.println("Fast Charging");
    }
    else {
        tft.fillRect(120,90,100,30,ILI9341_NAVY);
        tft.fillRect(20,90,100,30,ILI9341_GREEN);
        Serial.println("Slow Charging");
    }
    digitalWrite(blueLED, fastCharge);
    delay(200);
}

void setChargeLimit(int8_t percentChange) {
    if (percentChange > 0) {
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
    }
    else if (percentChange < 0) {
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
    }
    chargeLimit += percentChange;
    EEPROM.update(1, chargeLimit);
    if (chargeLimit == 100) {
        tft.setCursor(95, 195);
    }
    else {
        tft.setCursor(105, 195);
    }
    tft.print(chargeLimit);
}

void showBatteryPercentage(){
  tft.fillRect(0, 0, 240, 30, ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(165, 15);
  tft.print(percent);
  tft.print("%");
}