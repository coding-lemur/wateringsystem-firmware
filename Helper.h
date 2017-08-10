#ifndef Helper_h
#define Helper_h

#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include <RtcDS1307.h>

class Helper {
  public:
    static String getFormatedDateTime(const RtcDateTime& dt);
    static const char* floatToCharArray(float value);
    static String byteArrayToString(byte* bytes, unsigned int length);
};
#endif
