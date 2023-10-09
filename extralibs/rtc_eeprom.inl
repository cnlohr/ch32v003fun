//
// rtc_eeprom
//
// Copyright (c) 2019 BitBank Software, Inc.
// Written by Larry Bank
// bitbank@pobox.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "rtc_eeprom.h"

static int iRTCType;
static int iRTCAddr;

//
// Turn on the RTC
// returns 1 for success, 0 for failure
//
int rtcInit(int iType, int iSDA, int iSCL)
{
uint8_t ucTemp[4];

  if (iType <= RTC_UNKNOWN || iType >= RTC_TYPE_COUNT) // invalid type
     return 0;
  iRTCType = iType;
  if (iRTCType == RTC_DS3231)
     iRTCAddr = RTC_DS3231_ADDR;
  else if (iRTCType == RTC_RV3032)
     iRTCAddr = RTC_RV3032_ADDR;
//  else
//     iRTCAddr = RTC_PCF8563_ADDR;

  I2CInit(iSDA, iSCL, 50000); // initialize the bit bang library
  if (iType == RTC_DS3231) {
    ucTemp[0] = 0xe; // control register
    ucTemp[1] = 0x1c; // enable main oscillator and interrupt mode for alarms
    I2CWrite(iRTCAddr, ucTemp, 2);
  } else if (iType == RTC_RV3032) {
    // Enable direct switchover mode to the backup battery (disabled on delivery)
    ucTemp[0] = 0xc0; // EEPROM PMU
    ucTemp[1] = 0x10; // enable direct VBACKUP switchover, disable trickle charge
    I2CWrite(iRTCAddr, ucTemp, 2);
  }
//  else { // PCF8563
//    ucTemp[0] = 0; // control_status_1
//    ucTemp[1] = 0; // normal mode, clock on, power-on-reset disabled
//    ucTemp[2] = 0; // disable all alarms
//    I2CWrite(iRTCAddr, ucTemp, 3);
//  }
  return 1;
} /* rtcInit() */
//
// Enable/set the CLKOUT frequency (-1 = disable)
//
void rtcSetFreq(int iFreq)
{
uint8_t c, ucTemp[4];
int i;

   if (iRTCType == RTC_RV3032) {
      if (iFreq == -1) { // disable it
          I2CRead(0xc0, &ucTemp[1], 1); // read control register
          ucTemp[0] = 0xc0; // write it back with NCLKE set to disable CLKOUT
          ucTemp[1] |= 0x40; // set NCLKE
          I2CWrite(iRTCAddr, ucTemp, 2);
      } else { // enable clock
          I2CRead(0xc0, &ucTemp[1], 1); // read control register
          ucTemp[0] = 0xc0; // write it back with NCLKE set to disable CLKOUT
          ucTemp[1] &= ~0x40; // clear NCLKE
          I2CWrite(iRTCAddr, ucTemp, 2);
          c = 0; // default = 32768
          if (iFreq <= 32768) { // low speed
             ucTemp[0] = 0xc3; // CLKOUT control
             if (iFreq == 1024) c = 1;
             else if (iFreq == 64) c = 2;
             else if (iFreq == 1) c = 3; // all other values will stay at 32k
             ucTemp[1] = c << 5; // bits 5+6 in 32k mode
             I2CWrite(iRTCAddr, ucTemp, 2);
          } else { // high speed
             ucTemp[0] = 0xc2; // HFD + CLKOUT control
             i = (iFreq / 8192000) - 1;
             if (i < 0) i = 0;
             else if (i > 8191) i = 8191; // top 13 bits of freq up to 67Mhz
             ucTemp[1] = (uint8_t)(i & 0xff);
             ucTemp[2] = (uint8_t)(0x80 | ((i >> 8) & 0x1f));
             I2CWrite(iRTCAddr, ucTemp, 3);
          }
      }
   } else if (iRTCType == RTC_DS3231) {
       if (iFreq == -1) { // disable CLKOUT (allow interrupts)
          ucTemp[0] = 0xe;// control register
          ucTemp[1] = 0x4; // disable SQW and enable interrupts 
       } else { // enable CLKOUT (disable interrupts)
          ucTemp[0] = 0xe;
          c = 3; // assume 8192Hz (default
          if (iFreq == 1) c = 0;
          else if (iFreq == 1024) c = 1;
          else if (iFreq == 4096) c = 2;
          else if (iFreq == 8192) c = 3;
          ucTemp[1] = 0x40 | (c << 3); // enable SQW, disable interrupts
       }
       I2CWrite(iRTCAddr, ucTemp, 2);
   }
 //  else if (iRTCType == RTC_PCF8563) {
 //  }
} /* rtcSetFreq() */
//
// Get the UNIX epoch time
// (only available on the RV-3032-C7
//
uint32_t rtcGetEpoch(void)
{
uint32_t tt = 0;

   if (iRTCType != RTC_RV3032)
      return tt;
   I2CRead(0x1b, (uint8_t *)&tt, sizeof(tt));
   return tt;
} /* rtcGetEpoch() */
//
// Set the UNIX epoch time
// (only available on the RV-3032-C7
//
void rtcSetEpoch(uint32_t tt)
{
uint8_t ucTemp[4];

  I2CRead(0x10, ucTemp, 1); // read control register 2
  ucTemp[0] |= 1; // set RESET BIT
  I2CWrite(0x10, ucTemp, 1); // do a reset of seconds and prescaler
  I2CWrite(0x1b, (uint8_t *)&tt, sizeof(tt)); // set time
} /* rtcSetEpoch() */

//
// Set Alarm for:
// ALARM_SECOND = Once every second
// ALARM_MINUTE = Once every minute
// ALARM_TIME = When a specific hour:second match
// ALARM_DAY = When a specific day of the week and time match
// ALARM_DATE = When a specific day of the month and time match
//
void rtcSetAlarm(uint8_t type, struct tm *pTime)
{
uint8_t ucTemp[8];

  if (iRTCType == RTC_DS3231)
  {
    switch (type)
    {
      case ALARM_SECOND: // turn on repeating alarm for every second
        ucTemp[0] = 0xe; // control register
        ucTemp[1] = 0x1d; // enable alarm1 interrupt
        I2CWrite(iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0x7; // starting register for alarm 1
        ucTemp[1] = 0x80; // set bit 7 in the 4 registers to tell it a repeating alarm
        ucTemp[2] = 0x80;
        ucTemp[3] = 0x80;
        ucTemp[4] = 0x80;
        I2CWrite(iRTCAddr, ucTemp, 5);
        break;
      case ALARM_MINUTE: // turn on repeating alarm for every minute
        ucTemp[0] = 0xe; // control register
        ucTemp[1] = 0x1e; // enable alarm2 interrupt
        I2CWrite(iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0xb; // starting register for alarm 2
        ucTemp[1] = 0x80; // set bit 7 in the 3 registers to tell it a repeating alarm
        ucTemp[2] = 0x80;
        ucTemp[3] = 0x80;
        I2CWrite(iRTCAddr, ucTemp, 4);
        break;
      case ALARM_TIME: // turn on alarm to match a specific time
      case ALARM_DAY: // turn on alarm for a specific day of the week
      case ALARM_DATE: // turn on alarm for a specific date
// Values are stored as BCD
        ucTemp[0] = 0x7; // start at register 7
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        if (type == ALARM_DAY)
           ucTemp[4] = pTime->tm_wday + 1;
        // day of the month
        else if (type == ALARM_DATE) {
          ucTemp[4] = (pTime->tm_mday / 10) << 4;
          ucTemp[4] |= (pTime->tm_mday % 10);
        } else {
          ucTemp[4] = 0;
        }
        // set the A1Mx bits (high bits of the 4 registers)
        // for the specific type of alarm
        ucTemp[1] &= 0x7f; // make sure A1M1 & A1M2 are set to 0
        ucTemp[2] &= 0x7f;
        if (type == ALARM_TIME) // A1Mx bits should be x1000
        {
          ucTemp[3] &= 0x7f;
          ucTemp[4] |= 0x80;
        }
        else if (type == ALARM_DAY) // A1Mx bits should be 10000
        {
          ucTemp[3] &= 0x7f;
          ucTemp[4] &= 0x7f;
          ucTemp[4] |= 0x40; // DY/DT bit
        }
        // for matching the date, all bits are left as 0's (00000)
        I2CWrite(iRTCAddr, ucTemp, 5);
        ucTemp[0] = 0xe; // control register
        ucTemp[1] = 0x5; // enable alarm1 interrupt
        ucTemp[2] = 0x00; // reset alarm status bits
        I2CWrite(iRTCAddr, ucTemp, 3);
        break;
     } // switch on type
  }
#ifdef FUTURE
  else if (iRTCType == RTC_PCF8563)
  {
    switch (type)
    {
      case ALARM_SECOND: // turn on repeating alarm for every second
        ucTemp[0] = 0x1; // control_status_2
        ucTemp[1] = 0x5; // enable timer & interrupt
        I2CWrite(iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0xe; // timer control
        ucTemp[1] = 0x81; // enable timer for 1/64 second interval
        ucTemp[2] = 0x40; // timer count value (64 = 1 second)
        I2CWrite(iRTCAddr, ucTemp, 3);
        break;
      case ALARM_MINUTE: // turn on repeating timer for every minute
        ucTemp[0] = 0x1; // control_status_2
        ucTemp[1] = 0x5; // enable timer & interrupt
        I2CWrite(iRTCAddr, ucTemp, 2);
        ucTemp[0] = 0xe; // timer control
        ucTemp[1] = 0x82; // enable timer for 1 hz interval
        ucTemp[2] = 0x3c; // 60 = 1 minute
        I2CWrite(iRTCAddr, ucTemp, 3);
        break;
      case ALARM_TIME: // turn on alarm to match a specific time
      case ALARM_DAY: // turn on alarm for a specific day of the week
      case ALARM_DATE: // turn on alarm for a specific date
        ucTemp[0] = 0x1; // control_status_2
        ucTemp[1] = 0xa; // enable alarm & interrupt
        I2CWrite(iRTCAddr, ucTemp, 2);
// Values are stored as BCD
        ucTemp[0] = 0x9; // start at register 9
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        ucTemp[1] |= 0x80; // disable
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        ucTemp[2] |= 0x80; // disable
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        ucTemp[3] |= 0x80; // disable
        // day of the week
        ucTemp[5] = pTime->tm_wday + 1;
        ucTemp[5] = 0x80; // disable
        // day of the month
        ucTemp[4] = (pTime->tm_mday / 10) << 4;
        ucTemp[4] |= (pTime->tm_mday % 10);
        ucTemp[4] |= 0x80; // disable
        // clear high bits of the 4 registers
        // for the specific type of alarm
        if (type == ALARM_TIME)
        {
          ucTemp[1] &= 0x7f;
          ucTemp[2] &= 0x7f;
          ucTemp[3] &= 0x7f;
        }
        else if (type == ALARM_DAY)
        {
          ucTemp[5] &= 0x7f;
        }
        else if (type == ALARM_DATE)
        {
          ucTemp[4] &= 0x7f;
        }
        I2CWrite(iRTCAddr, ucTemp, 6);
        break;
     } // switch on alarm type
   } // PCF8563
#endif // FUTURE
  else if (iRTCType == RTC_RV3032) {
     switch (type)
     {
        //case ALARM_SECOND: // not supported
        case ALARM_MINUTE: // repeats every minute (special case of all 3 disabled)
           ucTemp[0] = 0x08; // minutes alarm
           ucTemp[1] = 0x80; // disable minutes alarm
           ucTemp[2] = 0x80; // disable hours alarm
           ucTemp[3] = 0x80; // disable date alarm
           I2CWrite(iRTCAddr, ucTemp, 4);
           break;
        case ALARM_TIME:
            //case ALARM_DAY: // not supported
        case ALARM_DATE:
               ucTemp[0] = 0x08; // minutes alarm
               ucTemp[1] = ((pTime->tm_min / 10) << 4);
               ucTemp[1] |= (pTime->tm_min % 10); // first 7 bits hold BCD minutes
               ucTemp[2] = ((pTime->tm_hour / 10) << 4);
               ucTemp[2] |= (pTime->tm_hour % 10);
               if (type == ALARM_TIME) {
                  ucTemp[3] = 0x80; // disable date alarm
               } else {
                  ucTemp[3] = ((pTime->tm_mday+1) / 10) << 4;
                  ucTemp[3] |= ((pTime->tm_mday+1) % 10);
               }
               I2CWrite(iRTCAddr, ucTemp, 4);
               break;
         } // switch on alarm type
         ucTemp[0] = 0x11; // Control 2
         ucTemp[1] = 0x08; // enable time interrupt and disable other int functions
         I2CWrite(iRTCAddr, ucTemp, 2);
      } // RV3032
} /* rtcSetAlarm() */
//
// Read the current internal temperature
// Value is celcius * 4 (resolution of 0.25C)
//
int rtcGetTemp(void)
{
unsigned char ucTemp[2];
int iTemp = 0;

  if (iRTCType == RTC_DS3231) {
    I2CReadRegister(iRTCAddr, 0x11, ucTemp, 2); // MSB location
    iTemp = ucTemp[0] << 8; // high byte
    iTemp |= ucTemp[1]; // low byte
    iTemp >>= 6; // lower 2 bits are fraction; upper 8 bits = integer part
  } else if (iRTCType == RTC_RV3032) {
    I2CReadRegister(iRTCAddr, 0x0e, ucTemp, 2); // LSB, then MSB
    iTemp = ucTemp[0] | (ucTemp[1] << 8);
    iTemp >>= 6; // lower 2 bits are fraction upper 8 are integer
  }
  return iTemp; // no temperature sensor
} /* rtcGetTemp() */
//
// Set the current time/date
//
void rtcSetTime(struct tm *pTime)
{
unsigned char ucTemp[20];
uint8_t i;

   if (iRTCType == RTC_DS3231) {
// Values are stored as BCD
        ucTemp[0] = 0; // start at register 0
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        ucTemp[4] = pTime->tm_wday + 1;
        // day of the month
        ucTemp[5] = (pTime->tm_mday / 10) << 4;
        ucTemp[5] |= (pTime->tm_mday % 10);
        // month + century
        i = pTime->tm_mon+1; // 1-12 on the RTC
        ucTemp[6] = (i / 10) << 4;
        ucTemp[6] |= (i % 10);
        if (pTime->tm_year >= 100)
           ucTemp[6] |= 0x80; // century bit
        // year
        ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
        ucTemp[7] |= (pTime->tm_year % 10);
#ifdef FUTURE
    } else if (iRTCType == RTC_PCF8563) {
        ucTemp[0] = 2; // start at register 2
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours (and set 24-hour format)
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        ucTemp[5] = pTime->tm_wday + 1;
        // day of the month
        ucTemp[4] = (pTime->tm_mday / 10) << 4;
        ucTemp[4] |= (pTime->tm_mday % 10);
        // month + century
        i = pTime->tm_mon+1; // 1-12 on the RTC
        ucTemp[6] = (i / 10) << 4;
        ucTemp[6] |= (i % 10);
        if (pTime->tm_year >= 100)
           ucTemp[6] |= 0x80; // century bit
        // year
        ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
        ucTemp[7] |= (pTime->tm_year % 10);
#endif // FUTURE
    } else if (iRTCType == RTC_RV3032) {
// Values are stored as BCD
        ucTemp[0] = 1; // start at register 1
        // seconds
        ucTemp[1] = ((pTime->tm_sec / 10) << 4);
        ucTemp[1] |= (pTime->tm_sec % 10);
        // minutes
        ucTemp[2] = ((pTime->tm_min / 10) << 4);
        ucTemp[2] |= (pTime->tm_min % 10);
        // hours
        ucTemp[3] = ((pTime->tm_hour / 10) << 4);
        ucTemp[3] |= (pTime->tm_hour % 10);
        // day of the week
        ucTemp[4] = pTime->tm_wday + 1;
        // day of the month
        ucTemp[5] = (pTime->tm_mday / 10) << 4;
        ucTemp[5] |= (pTime->tm_mday % 10);
        // month
        i = pTime->tm_mon+1; // 1-12 on the RTC
        ucTemp[6] = (i / 10) << 4;
        ucTemp[6] |= (i % 10);
        // year
        ucTemp[7] = (((pTime->tm_year % 100)/10) << 4);
        ucTemp[7] |= (pTime->tm_year % 10);
    }
    I2CWrite(iRTCAddr, ucTemp, 8);

} /* rtcSetTime() */

//
// Read the current time/date
//
void rtcGetTime(struct tm *pTime)
{
unsigned char ucTemp[20];

  if (iRTCType == RTC_DS3231) {
        I2CReadRegister(iRTCAddr, 0, ucTemp, 7); // start of data registers
        memset(pTime, 0, sizeof(struct tm));
        // convert numbers from BCD
        pTime->tm_sec = ((ucTemp[0] >> 4) * 10) + (ucTemp[0] & 0xf);
        pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
        // hours are stored in 24-hour format in the tm struct
        if (ucTemp[2] & 64) // 12 hour format
        {
                pTime->tm_hour = ucTemp[2] & 0xf;
                pTime->tm_hour += ((ucTemp[2] >> 4) & 1) * 10;
                pTime->tm_hour += ((ucTemp[2] >> 5) & 1) * 12; // AM/PM
        }
        else // 24 hour format
        {
                pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);
        }
        pTime->tm_wday = ucTemp[3] - 1; // day of the week (0-6)
        // day of the month
        pTime->tm_mday = ((ucTemp[4] >> 4) * 10) + (ucTemp[4] & 0xf);
        // month
        pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11
        pTime->tm_year = (ucTemp[5] >> 7) * 100; // century
        pTime->tm_year += ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);
  } else if (iRTCType == RTC_PCF8563) {
        I2CReadRegister(iRTCAddr, 2, ucTemp, 7); // start of data registers
        memset(pTime, 0, sizeof(struct tm));
        // convert numbers from BCD
        pTime->tm_sec = (((ucTemp[0] >> 4) & 7) * 10) + (ucTemp[0] & 0xf);
        pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
        // hours are stored in 24-hour format in the tm struct
        pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);
        pTime->tm_wday = ucTemp[4] - 1; // day of the week (0-6)
        // day of the month
        pTime->tm_mday = ((ucTemp[3] >> 4) * 10) + (ucTemp[3] & 0xf);
        // month
        pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11
        pTime->tm_year = (ucTemp[5] >> 7) * 100; // century
        pTime->tm_year += ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);
  } else if (iRTCType == RTC_RV3032) {
        I2CReadRegister(iRTCAddr, 0x01, ucTemp, 7); // start of data registers
        memset(pTime, 0, sizeof(struct tm));
        // convert numbers from BCD
        pTime->tm_sec = ((ucTemp[0] >> 4) * 10) + (ucTemp[0] & 0xf);
        pTime->tm_min = ((ucTemp[1] >> 4) * 10) + (ucTemp[1] & 0xf);
        pTime->tm_hour = ((ucTemp[2] >> 4) * 10) + (ucTemp[2] & 0xf);
        pTime->tm_wday = ucTemp[3] - 1; // day of the week (0-6)
        // day of the month
        pTime->tm_mday = ((ucTemp[4] >> 4) * 10) + (ucTemp[4] & 0xf);
        // month
        pTime->tm_mon = (((ucTemp[5] >> 4) & 1) * 10 + (ucTemp[5] & 0xf)) -1; // 0-11     
        pTime->tm_year = 100 + ((ucTemp[6] >> 4) * 10) + (ucTemp[6] & 0xf);
  }
} /* rtcGetTime() */
//
// Set countdown alarm in milliseconds
//
void rtcCountdownAlarm(uint32_t u32Millis)
{
uint8_t ucTemp[4];
int iTicks;
uint8_t ucClkDiv;

	if (u32Millis < 1000) { // use fastest clock divider (1/4096 sec)
		ucClkDiv = 0; // TD = 00
		iTicks = u32Millis * 4; // convert to ticks
	} else if (u32Millis < 64000) { // use 1/64 sec divider
		ucClkDiv = 1; // TD = 01
		iTicks = ((u32Millis * 64) / 1000);
	} else if (u32Millis < 4095000) { // use 1 sec divider
		ucClkDiv = 2; // TD = 10
		iTicks = u32Millis / 1000; // 1 second per tick
	} else { // one minute per division
		ucClkDiv = 3; // TD = 11
		iTicks = u32Millis / 60000; // 1 minute per tick
	}
	ucTemp[0] = 0xb; // low byte of countdown timer
	ucTemp[1] = (uint8_t)iTicks;
	ucTemp[2] = (uint8_t)(iTicks >> 8);
	I2CWrite(iRTCAddr, ucTemp, 3);
	// set up the clock frequency to use seconds as the period
	I2CReadRegister(iRTCAddr, 0x10, &ucTemp[1], 3); // control reg 1/2/3
	ucTemp[1] &= 0xfc; // control 1
	ucTemp[1] |= (0x08 | ucClkDiv); // enable TE (period countdown timer), set TD
	ucTemp[2] &= ~0x2c; // disable periodic/alarm and external interrupts
	ucTemp[2] |= 0x10; // enable countdown interrupt
	ucTemp[3] = 0; // disable backup switchover and all temperature interrupts
	ucTemp[0] = 0x10; // write all 3 control registers back
	I2CWrite(iRTCAddr, ucTemp, 4); // start countdown timer
} /* rtcCountdownAlarm() */
//
// Reset the "fired" bits for Alarm 1 and 2
// Interrupts will not occur until these bits are cleared
//
void rtcClearAlarms(void)
{
uint8_t ucTemp[4];

  if (iRTCType == RTC_DS3231)
  {
    ucTemp[0] = 0xe; // control register
    ucTemp[1] = 0x4; // disable alarm interrupt bits
    ucTemp[2] = 0x0; // clear A1F & A2F (alarm 1 or 2 fired) bit to allow it to fire again
    I2CWrite(iRTCAddr, ucTemp, 3);
  }
  else if (iRTCType == RTC_PCF8563)
  {
    ucTemp[0] = 1; // control_status_2
    ucTemp[1] = 0; // disable all alarms
    I2CWrite(iRTCAddr, ucTemp, 2);
  }
  else if (iRTCType == RTC_RV3032)
  {
//	    I2CReadRegister(iRTCAddr, 0x11, &ucTemp[1], 1);
//	    ucTemp[0] = 0x11; // control 2
//	    ucTemp[1] &= 0x81; // disable all alarms
//	    I2CWrite(iRTCAddr, ucTemp, 2);
	    ucTemp[0] = 0x0d; // status register
	    ucTemp[1] = 0x00; // clear all flags
	    I2CWrite(iRTCAddr, ucTemp, 2);
  }
} /* rtcClearAlarms() */

