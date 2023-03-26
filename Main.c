#include <RTClib.h>
#include <Wire.h>
#include "LTC2941.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define Serial SerialUSB
#else
    #define Serial Serial
#endif

#define aref_voltage 3.3                                            //ARef pin is tied to 3.3V to decrease noise in TMP36 temperature reading

int PWMpin = 4;                                                     //Pin 4 used due to higher base PWM frequency of 980Hz
int tempPin = A1;                                                   //TMP36's Analog Vout (Sense) pin is connected to pin A1 on Arduino

RTC_DS1307 rtc;                                                     //Declares RTC object
DateTime currentTime;
uint8_t alarmHour;
uint8_t alarmMinute;
bool isPM;
uint8_t alarmHourDiff;
uint8_t alarmMinuteDiff;

//float coulomb = 0;
float mAh = 0;
float percent = 0;
float prevBatteryTemp = 0;
//float currentBatteryTemp = 0;
int dutyCycle = 0;
int dutyCycleTempLoss = 0;                                          //Stores duty cycle decrease due to temperature limit

//post-testing table of duty cycle values vs. battery percentage/voltage (3-7-4.2V)

void setup(void) {
    TCCR0B = TCCR0B & B11111000 | B00000010;                        //Sets PWM switching frequency to 7812.5Hz for pins 4/13
    pinMode(PWMpin, OUTPUT);

    noInterrupts();                                                 //Temporarily disables all interrupts
    TCCR1A = 0;                                                     //Initializes 16-bit Timer1
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A = 62500;                                                  //Sets Output Compare Register (16MHz clock/1024 prescaler/0.25Hz timer)
    TCCR1B |= (1 << WGM12);                                         //CTC mode clears timer when timer counter reaches OCR register value
    TCCR1B |= (1 << CS12)|(1 << CS10);                              //Sets prescaler to 1024
    TIMSK1 |= (1 << OCIE1A);                                        //Enables timer compare interrupt
    interrupts();                                                   //Enables all interrupts

    Wire.begin();
    while (!rtc.begin());                                           //Test for successful connection to DS1307
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                 //Sets RTC to the date & time on the computer when sketch was compiled

    Serial.begin(115200);
    while (!Serial.available());
    //Serial.println("LTC2941 Raw Data");
    ltc2941.initialize();
    ltc2941.setBatteryFullMAh(1000);
}

ISR(TIMER1_COMPA_vect) {                                            //Timer1 compare interrupt service routine
    //currentBatteryTemp = getBatteryTemp();                        //Reads current battery temperature
    int tempReading = analogRead(tempPin);                          //Reads in and converts TMP36's Vout to int between 0-1023
    float tempVoltage = tempReading * aref_voltage / 1023.0;        //Converts Vout from int to voltage between 0-3.3V
    float currentBatteryTemp = (tempVoltage - 0.5) * 100;           //Converts voltage to °C with 10mV/°C and 500mV offset
    if (currentBatteryTemp > 45) {
        if (currentBatteryTemp >= prevBatteryTemp) {                //If battery temp exceeds 45°C limit and is not decreasing
            //analogWrite(PWMpin, dutyCycle[getBatteryPercent] - ++dutyCycleTempLoss);   //Decrease PWM duty cycle/charging current and record that duty cycle has been temporarily decreased
        }
    }
    else {
        if (dutyCycleTempLoss > 0) {                                //If battery temp is under 45°C limit and duty cycle has been temporarily decreased
            //analogWrite(PWMpin, dutyCycle[getBatteryPercent] - --dutyCycleTempLoss);   //Increase duty cycle until it is reaches normal values
        }
    }
    prevBatteryTemp = currentBatteryTemp;
}

void loop(void) {
    //coulomb = ltc2941.getCoulombs();
    mAh = ltc2941.getmAh();
    percent = ltc2941.getPercent();
    // Serial.print(coulomb);
    // Serial.print("C,");
    // Serial.print(mAh);
    // Serial.print("mAh,");
    // Serial.print(percent);
    // Serial.print("%");
    // Serial.println();
    delay(1000);

    //if (setAlarmButton.pressed)
        currentTime = RTC.now;
        alarmHourDiff = (alarmHour >= currentTime.hour() ? 0 : 24) + alarmHour - currentTime.hour() - (alarmMinute < currentTime.minute() ? 1 : 0);
        alarmMinuteDiff = (alarmMinute >= currentTime.minute() ? 0 : 60) + alarmMinute - currentTime.minute();
}

// float getBatteryTemp(){
//     int tempReading = analogRead(tempPin);                          //Reads in and converts TMP36's Vout to int between 0-1023
//     float tempVoltage = tempReading * aref_voltage / 1023.0;        //Converts Vout from int to voltage between 0-3.3V
//     float tempC = (tempVoltage - 0.5) * 100;                        //Converts voltage to °C with 10mV/°C and 500mV offset
//     return tempC;
// }