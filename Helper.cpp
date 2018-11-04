#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include "Helper.h"

String Helper::byteArrayToString(byte* bytes, unsigned int length) {
  String result = "";
  
  for (int i = 0; i < length; i++) {
    result += (char)bytes[i];
  }

  return result;
}

