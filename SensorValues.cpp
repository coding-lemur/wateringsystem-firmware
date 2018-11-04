#if ARDUINO < 100
#include <WProgramm.h>
#else
#include <Arduino.h>
#endif

#include "SensorValues.h"

SensorValues::SensorValues() :
  _isEmpty(true) {
}

SensorValues::SensorValues(float temperature, float humidity, float pressure, int soilMoisture) :
  _isEmpty(false),
  _temperature(temperature),
  _humidity(humidity),
  _pressure(pressure),
  _soilMoisture(soilMoisture) {
}

float SensorValues::getTemperature() {
  return _temperature;
}

float SensorValues::getHumidity() {
  return _humidity;
}

float SensorValues::getPressure() {
  return _pressure;
}

int SensorValues::getSoilMoisture() {
  return _soilMoisture;
}

boolean SensorValues::isEmpty() {
  return _isEmpty;
}

