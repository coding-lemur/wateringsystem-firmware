#ifndef Watering_h
#define Watering_h

#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

class Watering {
  public:
    Watering(uint8_t pumpActivatePin);
    void startPump(int milliseconds);
    void setup();
    void loop();
    bool isWatering();
    void setCallback(void (*callback)(char*));
    
  private:
    uint8_t _pumpActivatePin;
    bool _isWatering;
    unsigned long _pumpStopMillis;
    void (*_callback)(char*);

    void _stopPump();
};
#endif
