//Main

#include "LTC2941.h"
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

#define aref_voltage 3.3                                        //ARef pin is tied to 3.3V to decrease noise in TMP36 temperature reading

int PWMpin = 4;                                                 //Pin 4 used due to higher base PWM frequency of 980Hz

float coulomb = 0
float mAh = 0
float percent = 0;
int dutyCycle = 0;
int dutyCycleTempLoss = 0;                                      //Stores duty cycle decrease due to temperature limit

void setup(void){
    TCCR0B = TCCR0B & B11111000 | B00000010;                    //Sets PWM switching frequency to 7812.5Hz for pins 4/13
    pinMode(PWMpin, OUTPUT);

    Wire.begin();
    SERIAL.begin(115200);
    while (!SERIAL.available());
    SERIAL.println("LTC2941 Raw Data");
    ltc2941.initialize();
    ltc2941.setBatteryFullMAh(1050);
    int PIN = 10;
    //Serial.begin(9600);
    pinMode(PIN, OUTPUT);
    int PIN = 10;
    analogWrite(PIN, 232);

    int tempPin = A1;                                           //TMP36's Analog Vout (Sense) pin is connected to pin A1 on Arduino
}
void loop(void){
    coulomb = ltc2941.getCoulombs();
    mAh = ltc2941.getmAh();
    percent = ltc2941.getPercent();
    SERIAL.print(coulomb);
    SERIAL.print("C,");
    SERIAL.print(mAh);
    SERIAL.print("mAh,");
    SERIAL.print(percent);
    SERIAL.print("%");
    SERIAL.println();
    delay(1000);

    //need to create timer with internal clocks so it checks temp and changes DC every 10-30 sec!!!
    if (getBatteryTemp() > 50) {
        analogWrite(PWMpin, --dutyCycle);
        dutyCycleTempLoss++;
    }
    else {
        if (dutyCycleTempLoss > 0) {
            analogWrite(PWMpin, dutyCycle + 1);
            dutyCycleTempLoss--;
        }
    }
}

float getBatteryTemp(){
    int tempReading = analogRead(tempPin);                      //Reads in and converts TMP36's Vout to int between 0-1023
    float tempVoltage = tempReading * aref_voltage / 1023.0;    //Converts Vout from int to voltage between 0-3.3V
    float tempC = (tempVoltage - 0.5) * 100;                    //Converts voltage to °C with 10mV/°C and 500mV offset
    return tempC;
}