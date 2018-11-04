#ifndef Helper_h
#define Helper_h

#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

class Helper {
  public:
    static String byteArrayToString(byte* bytes, unsigned int length);
};
#endif
