#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include "Watering.h"

Watering::Watering(uint8_t pumpActivatePin) {
  _pumpActivatePin = pumpActivatePin;

  _isWatering = false;
  _pumpStopMillis = 0;
}

void Watering::startPump(int milliseconds) {
  if (_isWatering) {
    return;
  }
  
  Serial.print("watering starting ");
  Serial.println(milliseconds);

  digitalWrite(_pumpActivatePin, HIGH);
  
  unsigned long currentMillis = millis();
  _pumpStopMillis = currentMillis + milliseconds;

  _isWatering = true;
}

void Watering::_stopPump() {
  if (!_isWatering) {
    return;
  }
  
  digitalWrite(_pumpActivatePin, LOW);

  _isWatering = false;
  
  Serial.println("watering finished");
}

void Watering::setup() {
  pinMode(_pumpActivatePin, OUTPUT);
  digitalWrite(_pumpActivatePin, LOW);
}

void Watering::loop() {
  if (!_isWatering) {
    return;
  }
  
  unsigned long currentMillis = millis();
  
  if (currentMillis >= _pumpStopMillis) {
    Watering::_stopPump();
  }
}

