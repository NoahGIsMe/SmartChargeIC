#include "LTC2941.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif

float coulomb = 0, mAh = 0, percent = 0;
uint16_t timeCount = 0;
int indexCount = 0;
uint16_t fastChargeLUT[101] = {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
                                   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0};

void setup(void)
{
    Wire.begin();
    
    SERIAL.begin(115200);
    while(!SERIAL.available());
    
    SERIAL.println("LTC2941 Raw Data");
    
    ltc2941.initialize();
    ltc2941.setBatteryFullMAh(1000, false);
}

void loop(void)
{
    percent = ltc2941.getPercent();
    if (percent >= indexCount) {
      fastChargeLUT[indexCount] = timeCount;
      Serial.println(timeCount);
      for(int i = 0; i < 101; i++) {
        Serial.print(fastChargeLUT[i]);
        Serial.print(", ");
      }
      Serial.print("\n");
      indexCount++;
    }
    if (percent >= 100) {
      while(true) {
        delay(10000);
      }
    }
    else {
      SERIAL.print(percent);
      SERIAL.println("%");
    }

    timeCount++;
    delay(1000);
}
