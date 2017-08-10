#ifndef DhtHelper_h
#define DhtHelper_h

#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include <DHT.h>

class DhtHelper {
  public:
    DhtHelper(uint8_t pin, uint8_t type);
    
    void begin();
    
    void refreshValues();
    
    float getTemperature();
    float getHumidity();
    
  private:
    DHT _dht;
    
    uint8_t _dht_pin;
    uint8_t _dht_type;
    
    float _temperature;
    float _humidity;
};
#endif
