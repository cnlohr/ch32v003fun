#ifndef __RTC_EEPROM__
#define __RTC_EEPROM__

// I2C base address of the DS3231 RTC and AT24C32 EEPROM
#define RTC_DS3231_ADDR 0x68
#define EEPROM_ADDR 0x57
#define RTC_RV3032_ADDR 0x51
#define RTC_PCF8563_ADDR 0x51

enum
{
  RTC_UNKNOWN=0,
  RTC_PCF8563,
  RTC_DS3231,
  RTC_RV3032,
  RTC_TYPE_COUNT
};

//
// Time structure
// modeled after Linux version
//
#ifndef _TIME_H_
struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};
#endif

// Alarm types
enum {
  ALARM_SECOND=0,
  ALARM_MINUTE,
  ALARM_TIME,
  ALARM_DAY,
  ALARM_DATE
};
//
// Turn on the RTC
// returns 1 for success, 0 for failure
//
int rtcInit(int iType, int iSDAPin, int iSCLPin);
//
// Enable/Set the CLKOUT frequency (-1 = disable)
//
void rtcSetFreq(int iFreq);
//
// Set Alarm for:
// ALARM_SECOND = Once every second
// ALARM_MINUTE = Once every minute
// ALARM_TIME = When a specific hour:second match
// ALARM_DAY = When a specific day of the week and time match
// ALARM_DATE = When a specific day of the month and time match
//
void rtcSetAlarm(uint8_t type, struct tm *thetime);
//
// Read the current internal temperature
// Value is celcius * 4 (resolution of 0.25C)
//
int rtcGetTemp(void);
//
// Set the current time/date
//
void rtcSetTime(struct tm *pTime);
//
// Read the current time/date
//
void rtcGetTime(struct tm *pTime);
//
// After an alarm triggers an interrupt, it sets a flag in register 0x0F
// If the flag is not cleared, no other interrupts will be generated
// This function clears the "fired" flags for both Alarm 1 and 2
//
void rtcClearAlarms(void);
//
// Set a periodic countdown alarm in milliseconds
//
void rtcCountdownAlarm(uint32_t u32Millis);

//
// Get the UNIX epoch time
// (only available on the RV-3032-C7
//
uint32_t rtcGetEpoch();
//
// Set the UNIX epoch time
// (only available on the RV-3032-C7
//
void rtcSetEpoch(uint32_t tt);
//
// Write a byte to the given address
// or the previous address if iAddr == -1
//
void eeWriteByte(int iAddr, unsigned char ucByte);
//
// Write a block of 32 bytes to the given address
// or from the last read/write address is iAddr == -1
//
void eeWriteBlock(int iAddr, unsigned char *pData);
//
// Read a byte from the EEPROM
//
void eeReadByte(int iAddr, unsigned char *pData);
//
// Read a block of 32 bytes from the given address
// or from the last read address if iAddr == -1
//
void eeReadBlock(int iAddr, unsigned char *pData);

#endif // __RTC_EEPROM__

