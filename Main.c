#include <Wire.h>       //I2C library
#include <EEPROM.h>     //EEPROM editor library
#include <RTClib.h>     //RTC library
#include "LTC2941.h"    //Coulomb counter library
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
#define TFT_DC 46                                                   //Can be any pin as long as its digital
#define TFT_CS 48
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
int8_t chargeLimit = 80;                                            //Sets default maximum battery percentage to 80%
bool alarmSet;
bool limitButton;
bool speedButton;

RTC_DS1307 rtc;                                                     //Declares RTC object
DateTime currentTime;
uint8_t alarmHour;
uint8_t alarmMinute;
bool isPM;
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
        else if () {
            //alarmSet
        }
        else if () {
            //alarmToggle
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
            if (currentBatteryTemp >= prevBatteryTemp)              //If battery temp exceeds 45°C limit and is not decreasing
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

void loadScreen() {
    tft.begin();
    Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
    TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
    
    //fastCharge = EEPROM.read(0);                                    //Loads charging speed setting from EEPROM
    //maxChargeLimit = EEPROM.read(1);                                //Loads maximum charge limit setting from EEPROM
    //alarmSet = EEPROM.read(2);

    //call print/updateBatteryPercentage function
    //call print/updateColorOfLightningBolt function if charging
    //print "Charging Mode", "Slow", and "Fast" in fixed positions
    //draw rounded rectangle in fixed position (https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives#rounded-rectangles-2002792)
    //call drawChargeSpeedCircle (fixed slow/fast positions)
    //print "Maximum Charge Limit" in fixed position
    //call print/updateChargeLimit function to print text
    //draw button that brings up numpad to enter charge limit
    //print "Alarm" in fixed position
    //call print/updateAlarmTime function
    //draw rounded rectangle in fixed position
    //call setAlarm function to draw circle position
}

void showNumpad() {

}

void setAlarm() {
    if (alarmSet == 1) {
        currentTime = rtc.now();
        alarmHourDiff = ((alarmHour >= currentTime.hour() ? 0 : 24) + alarmHour - currentTime.hour() - (alarmMinute < currentTime.minute() ? 1 : 0) + 24) % 24;
        alarmMinuteDiff = (alarmMinute >= currentTime.minute() ? 0 : 60) + alarmMinute - currentTime.minute();
    }
    //draw toggle button/circle (+ alarmSet * ~50pixelOffset)
        alarmSet ^= 1;
        EEPROM.update(2, alarmSet);
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
        tft.fillRect(20+100,90,100,30,ILI9341_BLUE);
        Serial.println("Fast Charging");
    }
    else {
        tft.fillRect(20+100,90,100,30,ILI9341_NAVY);                //20+100?
        tft.fillRect(20,90,100,30,ILI9341_GREEN);
        Serial.println("Slow Charging");
    }
    digitalWrite(blueLED, fastCharge);
    delay(200);                                                     //is this delay necessary? no delay in setchargeLimit
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
