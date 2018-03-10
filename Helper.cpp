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

String Helper::byteArrayToString(byte* bytes, unsigned int length) {
  String result = "";
  
  for (int i = 0; i < length; i++) {
    result += (char)bytes[i];
  }

  return result;
}

String Helper::getFormatedSeconds(unsigned long valueInSeconds) {
  //int days = elapsedDays(valueInSeconds);
  int hours = numberOfHours(valueInSeconds);
  int minutes = numberOfMinutes(valueInSeconds);
  int seconds = numberOfSeconds(valueInSeconds);

  return printDigits(hours) + ":" + printDigits(minutes) + ":" + printDigits(seconds);
}

// utility function for digital clock display: prints colon and leading 0
String Helper::printDigits(byte digits){
  String result = "";
  
  if(digits < 10)
    result += "0";
  
  return result + String(digits, DEC);
}

