#ifndef SensorValues_h
#define SensorValues_h

#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

class SensorValues {
  public:
    SensorValues(int temperature, int humidity, int soilMoisture);
    
    int getTemperature();
    int getHumidity();
    int getSoilMoisture();
    
  private:
    int _temperature;
    int _humidity;
    int _soilMoisture;
};
#endif
