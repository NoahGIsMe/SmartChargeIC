#include <Wire.h>       //I2C library
#include <EEPROM.h>     //EEPROM editor library
#include <RTClib.h>     //RTC library
#include "LTC2941.h"    //Coulomb counter library

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define Serial SerialUSB
#else
    #define Serial Serial
#endif

#define aref_voltage 3.3                                            //ARef pin is tied to 3.3V to decrease noise in TMP36 temperature reading
#define sensorPin A0

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
uint8_t maxChargeLimit;
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
    coulomb = ltc2941.getCoulombs();                                //reads charge in coulombs
    mAh = ltc2941.getmAh();                                         //reads charge in mAh
    percent = ltc2941.getPercent();                                 //reads battery percentage
    setAlarm();                                                     //set the alarm
    getBatteryTemp(void);                                           //get the battery temperature
    setChargeSpeed();                                               //set the charge speed
    setMaxChargeLimit();                                            //set the charge capacity
}

ISR(TIMER1_COMPA_vect) {                                            //Timer1 compare interrupt service routine
    if (++interruptCounter %= 10) {
        percent = ltc2941.getPercent();                             //call print/updateBatteryPercentage function - only update if it changes

        currentBatteryTemp = getBatteryTemp()                       //Reads current battery temperature
        int tempReading = analogRead(tempPin)                       //Reads in and converts TMP36's Vout to int between 0-1023
        float tempVoltage = tempReading * aref_voltage / 1023.0;    //Converts Vout from int to voltage between 0-3.3V
        float currentBatteryTemp = (tempVoltage - 0.5) * 100;       //Converts voltage to °C with 10mV/°C and 500mV offset
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
    fastCharge = EEPROM.read(0);                                    //Loads charging speed setting from EEPROM
    maxChargeLimit = EEPROM.read(1);                                //Loads maximum charge limit setting from EEPROM
    alarmSet = EEPROM.read(2);

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

void setChargeSpeed() {
    if (speedButton == true) {
        fastCharge ^= 1;
        digitalWrite(blueLED, HIGH);
        EEPROM.update(0, fastCharge);
    }
    else {
        digitalWrite(blueLED, LOW);
    }
}

void setMaxChargeLimit() {
    if (limitButton == true) {
        maxChargeLimit = userValueFromScreen
        EEPROM.update(1, maxChargeLimit);
    }
}
