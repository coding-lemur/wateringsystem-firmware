#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include "Helper.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

String Helper::getFormatedDateTime(const RtcDateTime& dt) {
  char datestring[20];
  
  snprintf_P(datestring,
          countof(datestring),
          PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
          dt.Month(),
          dt.Day(),
          dt.Year(),
          dt.Hour(),
          dt.Minute(),
          dt.Second() );
  
  return datestring;
}

/*
const char* Helper::floatToCharArray(float value) {
  char buffer[64];
  int bufferSize = sizeof buffer;
  
  String(value).toCharArray(buffer, bufferSize);
  
  return buffer;
}
*/

String Helper::byteArrayToString(byte* bytes, unsigned int length) {
  String result = "";
  
  for (int i = 0; i < length; i++) {
    result += (char)bytes[i];
  }

  return result;
}

