#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include "DhtHelper.h"

DhtHelper::DhtHelper(uint8_t pin, uint8_t type) : _dht(pin, type) {
  _dht_pin  = pin;
  _dht_type = type;
}

void DhtHelper::begin() {
  pinMode(_dht_pin, INPUT);
}

void DhtHelper::refreshValues() {
  byte retries = 0;
  
  do {
    _temperature = _dht.readTemperature();
    _humidity = _dht.readHumidity();
    
    if (isnan(_temperature) || isnan(_humidity)) {
      retries++;
  
      if (retries > 10) {
        break;
      }
      
      delay(250);
    }

  } while (isnan(_temperature) || isnan(_humidity));
}

float DhtHelper::getTemperature() {
  return _temperature;
}

float DhtHelper::getHumidity() {
  return _humidity;
}
