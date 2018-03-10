#ifndef Helper_h
#define Helper_h

#if ARDUINO < 100
  #include <WProgramm.h>
#else
  #include <Arduino.h>
#endif

#include <RtcDS1307.h>

// macros from DateTime.h 
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

class Helper {
  public:
    static String getFormatedDateTime(const RtcDateTime& dt);
    static String byteArrayToString(byte* bytes, unsigned int length);
    static String getFormatedSeconds(unsigned long valueInSeconds);

  private:
    static String printDigits(byte digits);
};
#endif
