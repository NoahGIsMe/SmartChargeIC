//Main

#include "LTC2941.h"
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

#define aref_voltage 3.3                                        //ARef pin is tied to 3.3V to decrease noise in TMP36 temperature reading

float coulomb = 0, mAh = 0, percent = 0;

void setup(void){
    Wire.begin();
    SERIAL.begin(115200);
    while (!SERIAL.available());
    SERIAL.println("LTC2941 Raw Data");
    ltc2941.initialize();
    ltc2941.setBatteryFullMAh(1050);
    int PIN = 10;
    Serial.begin(9600);
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
}

float getBatteryTemp(){
    int tempReading = analogRead(tempPin);                      //Reads in and converts TMP36's Vout to int between 0-1023
    float tempVoltage = tempReading * aref_voltage / 1023.0;    //Converts Vout from int to voltage between 0-3.3V
    float tempC = (tempVoltage - 0.5) * 100;                    //Converts voltage to °C with 10mV/°C and 500mV offset
}