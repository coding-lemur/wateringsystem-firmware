#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include "SensorValues.h"

SensorValues::SensorValues(int temperature, int humidity, int soilMoisture) :
  _temperature(temperature),
  _humidity(humidity),
  _soilMoisture(soilMoisture) {
}

int SensorValues::getTemperature() {
  return _temperature;
}

int SensorValues::getHumidity() {
  return _humidity;
}

int SensorValues::getSoilMoisture() {
  return _soilMoisture;
}
