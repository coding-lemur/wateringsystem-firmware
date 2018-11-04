#ifndef SensorValues_h
#define SensorValues_h

#if ARDUINO < 100
#include <WProgramm.h>
#else
#include <Arduino.h>
#endif

class SensorValues {
  public:
    SensorValues();
    SensorValues(float temperature, float humidity, float pressure, int soilMoisture);

    float getTemperature();
    float getHumidity();
    float getPressure();

    int getSoilMoisture();

    boolean isEmpty();

  private:
    bool _isEmpty;

    float _temperature;
    float _humidity;
    float _pressure;

    int _soilMoisture;
};
#endif
