//
// Portable Sensor Platform
// by Larry Bank
//
// A CH32V003 project to allow easy field-testing of various I2C sensors
// The FLASH memory (16K) doesn't allow too many sensors + display routines to be included
// but the ones supported are auto-detected and use a nice large font to display the information
//
//#define USE_IMU
//#define USE_RTC
//#define USE_BATT
//#define USE_BME

//#define BITBANG

#include "../../extralibs/ch32v_hal.inl"
#include "../../extralibs/sharp_lcd.inl"
#include "../../extralibs/ltr390.inl"
#include "../../extralibs/bme280.inl"
#include "Roboto_Black_40.h"
#include "../../extralibs/scd41.inl"
#include "../../extralibs/rtc_eeprom.inl"
#ifdef USE_IMU
#include "../../extralibs/LSM6DS3.inl"
#endif // USE_IMU
#include "battery.h"
//#include "isbic.h"
//#include "psp_logo.h"

struct tm myTime;
char szTemp[128];

// Hardware connections. The hex value represents the port (upper nibble) and GPIO pin (lower nibble)
// QFN20 PCB
//#define LCD_CS 0xd3
//#define LCD_VCOM 0xd4
// TSSOP20 PCB
#define LCD_CS 0xc7
#define LCD_VCOM 0xd3

#define LED_PIN 0xd6
#define SDA_PIN 0xc1
#define SCL_PIN 0xc2
#define BUTTON0_PIN 0xc3
#define BUTTON1_PIN 0xd5

int iSensor;
uint8_t bInvert = 0; // invert the LCD colors
extern uint32_t _iUV;

const char *szSensorNames[] = {"Unknown", "LTR390", "SCD4x", "LSM6DS3", "RV3032", "DS3231", "BME280"};

enum {
	SENSOR_UNKNOWN,
	SENSOR_LTR390,
	SENSOR_SCD4X,
	SENSOR_LSM6DS3,
	SENSOR_RV3032,
	SENSOR_DS3231,
        SENSOR_BME280,
	SENSOR_COUNT
};

int GetButtons(void)
{
        int i = 0;
//      pinMode(BUTTON0_PIN, INPUT_PULLUP); // re-enable gpio in case it got disabled by standby mode
//      pinMode(BUTTON1_PIN, INPUT_PULLUP);
        if (digitalRead(BUTTON0_PIN) == 0) i|=1;
        if (digitalRead(BUTTON1_PIN) == 0) i|=2;
        return i;

} /* GetButtons() */

// Convert a number into a zero-terminated string
int i2str(char *pDest, int iVal)
{
        char *d = pDest;
        int i, iPlaceVal = 1000000;
        int iDigits = 0;

        if (iVal < 0) {
                iDigits++;
                *d++ = '-';
                iVal = -iVal;
        }
        while (iPlaceVal) {
                if (iVal >= iPlaceVal) {
                        i = udiv32(iVal, iPlaceVal);
                        *d++ = '0' + (char)i;
                        iVal -= (i*iPlaceVal);
                        iDigits++;
                } else if (iDigits != 0) {
                        *d++ = '0'; // non-zeros were already displayed
                }
                iPlaceVal = udiv32(iPlaceVal, 10);
        }
        if (d == pDest) // must be zero
                *d++ = '0';
        *d++ = 0; // terminator
        return (int)(d - pDest - 1); // string length
} /* i2str() */

// Convert a number into a zero-terminated string
void i2strf(char *pDest, int iVal, int iDigits)
{
        char *d = pDest;
        int i;
        pDest[iDigits] = 0;
        while (iDigits) {
        	iDigits--;
            i = umod32(iVal, 10);
            d[iDigits] = '0' + (char)i;
            iVal = udiv32(iVal, 10);
        }
} /* i2strf() */

//
// Parse a zero-padded number (fixed length)
// from a string
int ParseInt(char *pString, int iLen)
{
int j, i = 0;
	for (j=0; j<iLen; j++) {
		i *= 10;
		if (pString[j] < '0' || pString[j] > '9') return -1; // invalid
		i += (pString[j] - '0');
	}
	return i;
} /* ParseInt() */
//
// Parse the $GNRMC string
//
int ParseGPSPos(char *pString, char *szLAT, char *szLONG)
{
int i;

    if (pString[7] < '0' || pString[7] > '9') return 0; // invalid
    i = ParseInt(&pString[7], 2); // get hours
    if (i < 0 || i > 23) return 0; // invalid
    if (pString[18] == 'A') { // position info is valid
    	memcpy(szLAT, &pString[20], 12);
    	szLAT[12] = 0;
    	memcpy(szLONG, &pString[33], 13);
    	szLONG[13] = 0;
    	return 1;
    }
    return 0;
} /* ParseGPSPos() */

//
// Parse a $GNZDA GPS time/date string and
// set our RTC with the info if it's valid
// returns 0 for invalid string, 1 for valid time, 2 for valid time+date
//
int ParseGPSTime(char *pTime, struct tm *pTM)
{
int i;

	if (memcmp(pTime, "$GNZDA", 6) == 0) { // start of time string
		if (pTime[7] < '0' || pTime[7] > '9')
			return 0; // invalid
		i = ParseInt(&pTime[7], 2); // get hours
		if (i < 0 || i > 23)
			return 0; // neither time nor date is valid
		pTM->tm_hour = i;
		pTM->tm_min = ParseInt(&pTime[9], 2); // get minutes
		pTM->tm_sec = ParseInt(&pTime[11], 2); // get seconds
		i = ParseInt(&pTime[18], 2); // get month day
		if (i < 1 || i > 31)
			return 1; // only time is valid
		pTM->tm_mday = i; // get month day
		pTM->tm_mon = ParseInt(&pTime[21], 2) - 1; // get month (fix for 0-11)
		pTM->tm_year = ParseInt(&pTime[24], 4) - 1900; // get year
		return 2; // both time and date are valid
	}
	return 0;
} /* ParseGPSTime() */
//
// Acquire the correct time and date
// from an external GPS module
//
#define UART_TIMEOUT 10000
int GPSTime(void)
{
char szLAT[16], szLONG[16];
int i, iLen = 0;
int iCount = 0;
int bHaveTime = 0;
struct tm myTime;

//	pinMode(LED_PIN, OUTPUT); // Show GPS is in use with LED blinking for data reception
	sharpFill(0);
	sharpWriteString(2,0, "GPS Parser", FONT_12x16, 0);
	sharpWriteBuffer();
	while (GetButtons() != 0) {}; // wait for user to release the button that got us here

    while (1) {
    	i = UART_Read(UART_TIMEOUT);
    	if (i != -1) {
    		digitalWrite(LED_PIN, 1);
    		if (i == 0xa) {
    			digitalWrite(LED_PIN, 0);
    			szTemp[iLen] = 0;
    			if (memcmp(szTemp, "$GNRMC", 6) == 0) {
    				i = ParseGPSPos(szTemp, szLAT, szLONG);
    				if (i >= 1) {
     				   sharpWriteString(2, 48, "LAT ", FONT_8x8, 0);
     				   sharpWriteString(-1, -1, szLAT, FONT_8x8, 0);
     				   sharpWriteString(2, 56, "LON ", FONT_8x8, 0);
     				   sharpWriteString(-1, -1, szLONG, FONT_8x8, 0);
     				   sharpWriteBuffer();
    				} else { // show the string that isn't valid yet
    					sharpWriteString(2, 56, szTemp, FONT_6x8, 0);
    					sharpWriteBuffer();
    				}
    			} // Position string
    			if (memcmp(szTemp, "$GNZDA", 6) == 0) {
    			   i = ParseGPSTime(szTemp, &myTime);
    			   if (i >= 1) { // time is valid
    				   bHaveTime = 1;
    				   sharpWriteString(2,16,"Time ", FONT_12x16, 0);
    				   i2strf(szTemp, myTime.tm_hour, 2);
    				   szTemp[2] = ':';
    				   i2strf(&szTemp[3], myTime.tm_min, 2);
    				   szTemp[5] = ':';
    				   i2strf(&szTemp[6], myTime.tm_sec, 2);
    				   sharpWriteString(-1, -1, szTemp, FONT_12x16, 0);
    				   sharpWriteBuffer();

    			   if (i == 2) { // date is also valid
    				   sharpWriteString(2,32,"Date ", FONT_12x16, 0);
    				   i2strf(szTemp, myTime.tm_mday, 2);
    				   szTemp[2] = '/';
    				   i = myTime.tm_mon+1;
    				   i2strf(&szTemp[3], i, 2);
    				   szTemp[5] = '/';
    				   i = myTime.tm_year % 100;
    				   i2strf(&szTemp[6], i, 2);
    				   sharpWriteString(-1, -1, szTemp, FONT_12x16, 0);
    				   sharpWriteBuffer();
//    				   rtcSetTime(&myTime);
//    				   while (GetButtons() == 0) {};
//    				   UART_DeInit();
//    				   return 1;
    			   }
    			}
    			} // string ended with 0x0A
    			if (bHaveTime == 0) {
    				iCount++;
    				sharpWriteString(2,32, szTemp, FONT_6x8, 0);
    				sharpWriteString(2,16,"Searching ", FONT_12x16, 0);
    				i2str(szTemp, iCount);
//    				sharpWriteString(-1, 16, szTemp, FONT_12x16, 0);
    				sharpWriteBuffer();
    			}
    			iLen = 0;
    		} else if (i >= ' ' && iLen < sizeof(szTemp)-1) {
    			szTemp[iLen++] = (char)i;
    		}
    	} else {
    	} // got a character
    } // while (1)
} /* GPSTime() */

void RunGPS(void)
{
    UARTInit(9600, 1); // enable remap to PC0/PC1
    GPSTime();
} /* RunGPS() */

#ifdef USE_BATT
void ADCInit(void)
{
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
    GPIOC->CFGLR &= ~(0xf<<(4*4));      // CNF = 00: Analog, MODE = 00: Input

    RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // ADCCLK = 24 MHz => RCC_ADCPRE divide by 2
    RCC->CFGR0 &= ~RCC_ADCPRE;  // Clear out the bis in case they were set
    RCC->CFGR0 |= RCC_ADCPRE_DIV2;      // set it to 010xx for /2.

    // turn on ADC and set rule group to sw trig
    ADC1->CTLR2 = ADC_ADON | ADC_EXTSEL;

    // Possible times: 0->3,1->9,2->15,3->30,4->43,5->57,6->73,7->241 cycles
    ADC1->SAMPTR2 = 0/*3 cycles*/ << (3/*offset per channel*/ * 2/*channel*/);

        // Set up single conversion on chl 2
//        ADC1->RSQR1 = 0;
//        ADC1->RSQR2 = 0;
        ADC1->RSQR3 = 2;        // 0-9 for 8 ext inputs and two internals

        // Reset calibration
        ADC1->CTLR2 |= ADC_RSTCAL;
        while(ADC1->CTLR2 & ADC_RSTCAL);

        // Calibrate
        ADC1->CTLR2 |= ADC_CAL;
        while(ADC1->CTLR2 & ADC_CAL);

} /* ADCInit() */

uint16_t ADCGetValue(void)
{
        // start sw conversion (auto clears)
        ADC1->CTLR2 |= ADC_SWSTART;

        // wait for conversion complete
        while(!(ADC1->STATR & ADC_EOC));

        // get result
        return (uint16_t)ADC1->RDATAR;

} /* ADCGetValue() */

void ShowBattery(int x, int y)
{
	int iBatt;
	uint8_t *s;

    ADCInit();
    iBatt = ADCGetValue();
    // turn off ADC
    ADC1->CTLR2 &= ~ADC_ADON;
i2str(szTemp, iBatt);
  sharpWriteString(2,60,szTemp, FONT_8x8, bInvert);
//	if (iBatt > 2000 && iBatt <2600) { // valid values, otherwise no battery measurement
if (0)
{
		if (iBatt <= 2216) s = (uint8_t *)batt0_32x16; // <= 3.6v
		else if (iBatt < 2345) s = (uint8_t *)batt25_32x16; // <= 3.8v
		else if (iBatt < 2410) s = (uint8_t *)batt50_32x16; // <= 3.9v
		else if (iBatt < 2466) s = (uint8_t *)batt75_32x16; // <= 4.0v
		else s = (uint8_t *)batt100_32x16;
		sharpDrawSprite(x, y, 32, 16, s, 4, 0);
	}
} /* ShowBattery() */
#endif // USE_BATT

#ifdef USE_RTC
void SetTime(void)
{
	int i, iFlash, iCount = 0, iTick = 0, iCursor = 0, bDone = 0;
	int iButts, iOldButts = 0;
	struct tm myTime;

	rtcGetTime(&myTime);

	while (!bDone) {
		iFlash = ((iTick & 15) > 3);
		sharpFill(bInvert);
		sharpWriteString(128,2,"Next", FONT_8x8, bInvert);
		sharpWriteString(152,58, "+", FONT_8x8, bInvert);
		sharpWriteString(16,2, "Set Time", FONT_12x16, bInvert);
	    i2strf(szTemp, myTime.tm_mday, 2);
	    if (iCursor != 0 || (iCursor == 0 && iFlash))
           sharpWriteString(0, 18, szTemp, FONT_12x16, bInvert);
	    sharpWriteString(24, 18, "/", FONT_12x16, bInvert);

	    i = myTime.tm_mon+1;
	    i2strf(szTemp, i, 2);
	    if (iCursor != 1 || (iCursor == 1 && iFlash))
           sharpWriteString(36, 18, szTemp, FONT_12x16, bInvert);
	    sharpWriteString(60, 18, "/", FONT_12x16, bInvert);

	    i = umod32(myTime.tm_year, 100);
	    i2strf(szTemp, i+2000, 2);
	    if (iCursor != 2 || (iCursor == 2 && iFlash))
           sharpWriteString(72, 18, szTemp, FONT_12x16, bInvert);

	    i2strf(szTemp, myTime.tm_hour, 2);
	    szTemp[2] = ':';
	    szTemp[3] = 0;
	    if (iCursor != 3 || (iCursor == 3 && iFlash))
           sharpWriteString(2, 34, szTemp, FONT_12x16, bInvert);
	    i2strf(szTemp, myTime.tm_min, 2);
	    szTemp[2] = ':';
	    szTemp[3] = 0;
	    if (iCursor != 4 || (iCursor == 4 && iFlash))
           sharpWriteString(38, 34, szTemp, FONT_12x16, bInvert);
	    i2strf(szTemp, myTime.tm_sec, 2);
	    if (iCursor != 5 || (iCursor == 5 && iFlash))
           sharpWriteString(74, 34, szTemp, FONT_12x16, bInvert);
	    // Check for user actions
	    iButts = GetButtons();
	    if (iButts == 0) iCount = 0; // reset "button held" counter
	    if (iButts == 3) { // user wants to exit without setting the time
	    	bDone = 1;
	    }
	    if (iCursor != 6 || (iCursor == 6 && iFlash))
	    	sharpWriteString(56, 50, "Done", FONT_12x16, bInvert);
	    if (iOldButts == 0 && iButts == 1) { // advance cursor
	    	iCursor++;
	    	if (iCursor > 6) iCursor = 0;
	    }
	    if (iButts == 2) {
	    	iCount++;
	    }
	    if (iCount == 1 || iCount > 24) { // adjust current (flashing) value
	    	switch (iCursor) {
	    	case 0: // day of the month
	    		myTime.tm_mday++;
	    		if (myTime.tm_mday > 31) myTime.tm_mday = 0;
	    		break;
	    	case 1: // month
	    		myTime.tm_mon++;
	    		if (myTime.tm_mon > 11) myTime.tm_mon = 0;
	    		break;
	    	case 2: // year
	    		myTime.tm_year++;
	    		myTime.tm_year = 100 + umod32(myTime.tm_year, 100);
	    		break;
	    	case 3: // hour
	    		myTime.tm_hour++;
	    		if (myTime.tm_hour > 23) myTime.tm_hour = 0;
	    		break;
	    	case 4: // minute
	    		myTime.tm_min++;
	    		if (myTime.tm_min > 59) myTime.tm_min = 0;
	    		break;
	    	case 5: // second
	    		myTime.tm_sec++;
	    		if (myTime.tm_sec > 59) myTime.tm_sec = 0;
	    		break;
	    	case 6:
	    		// set the time and leave
	    		rtcSetTime(&myTime);
	    		sharpFill(bInvert);
	            sharpWriteBuffer();
	    		return;
	    	}
	    }
	    iOldButts = iButts;
        sharpWriteBuffer();
        Delay_Ms(33);
        iTick++;
        if ((iTick & 31) == 31) {
        	// advance the time
        	myTime.tm_sec++;
        	if (myTime.tm_sec == 60) {
        		myTime.tm_sec = 0;
        		myTime.tm_min++;
        		if (myTime.tm_min == 60) {
        			myTime.tm_min = 0;
        			myTime.tm_hour++;
        			if (myTime.tm_hour == 24) {
        				myTime.tm_hour = 0;
        			}
        		}
        	}
        }
	} // while !bDone
	while (GetButtons()) {}; // wait for user to release all buttons
} /* SetTime() */

void ShowTime(void)
{

	sharpFill(bInvert);
	sharpWriteString(136, 2, "Set", FONT_8x8, bInvert);
	sharpWriteString(112, 58, "Invert", FONT_8x8, bInvert);
	rtcGetTime(&myTime);
   	i2strf(szTemp, myTime.tm_hour, 2);
   	szTemp[2] = ':';
   	i2strf(&szTemp[3], myTime.tm_min, 2);
   	sharpWriteStringCustom(&Roboto_Black_40, 2, 32, szTemp, !bInvert, 1);
   	i2strf(szTemp, myTime.tm_sec, 2);
   	sharpWriteString(112, 14, szTemp, FONT_12x16, bInvert);
   	i2strf(szTemp, myTime.tm_mday, 2);
   	szTemp[2] = '/';
   	i2strf(&szTemp[3], myTime.tm_mon+1, 2);
   	szTemp[5] = '/';
   	i2strf(&szTemp[6], myTime.tm_year + 1900, 4);
   	sharpWriteString(2, 36, szTemp, FONT_12x16, bInvert);
#ifdef USE_BATT
   	ShowBattery(2, 52);
#endif
   	sharpWriteBuffer();
} /* ShowTime() */
#endif // USE_RTC

void ShowLTR390Sample(int iValue, int iMax)
{
int iUVI;
int i;

    sharpFill(bInvert);
	sharpWriteString(24, 6, "UVI   Max", FONT_12x16, bInvert);
	iUVI = ltr390_getUVI(iValue); // instaneous value
	i2str(szTemp, udiv32(iUVI,10)); // whole part
    sharpWriteStringCustom(&Roboto_Black_40, 0, 62, szTemp, !bInvert, 0);
    sharpWriteStringCustom(&Roboto_Black_40, -1, 62, ".", !bInvert, 0);
	i = umod32(iUVI, 10); // 10ths
	i2str(szTemp, i);
    sharpWriteStringCustom(&Roboto_Black_40, -1, 62, szTemp, !bInvert, 0);

	iUVI = ltr390_getUVI(iMax); // max value from the last 3.2 seconds
	i2str(szTemp, udiv32(iUVI,10)); // whole part
    sharpWriteStringCustom(&Roboto_Black_40, 84, 62, szTemp, !bInvert, 0);
    sharpWriteStringCustom(&Roboto_Black_40, -1, 62, ".", !bInvert, 0);
	i = umod32(iUVI, 10); // 10ths
	i2str(szTemp, i);
    sharpWriteStringCustom(&Roboto_Black_40, -1, 62, szTemp, !bInvert, 0);
// DEBUG
//    i2str(szTemp, iValue);
//	sharpWriteString(2, 22, szTemp, FONT_8x8, 0);

	sharpWriteBuffer();
} /* ShowLTR390Sample() */

void i2hex(char *pStr, int i)
{
	char c;
	c = (i >> 4);
	if (c > 9) c += 55;
	else c += 48;
	pStr[0] = c;
	c = i & 0xf;
	if (c > 9)  c += 55;
	else c += 48;
	pStr[1] = c;
	pStr[2] = 0;
} /* i2hex() */

int GetSensorType(int i, char *szName)
{
uint8_t cTemp[4];
int iType = SENSOR_UNKNOWN;

	if (i == 0x53) // could be Lite-On LTR390 UV light sensor
	{
		I2CReadRegister(i, 0x06, cTemp, 1); // Part ID
		if (cTemp[0] == 0xb2) // a match!
			iType = SENSOR_LTR390;
	}
    if (i == 0x62 && iType == SENSOR_UNKNOWN) {
	    // DEBUG - for now, assume it's the SCD4x
	   iType = SENSOR_SCD4X;
    }
    if (iType == SENSOR_UNKNOWN && (i == 0x76 || i == 0x77)) {  // Check for BMP280/BME280
       I2CReadRegister(i, 0xd0, cTemp, 1); // WHOAMI register
       if (cTemp[0] == 0x60) // BME280
          iType = SENSOR_BME280;
    }
    if (i == 0x68 && iType == SENSOR_UNKNOWN) { // look for DS3231
        // Make sure it's really a DS3231 because other I2C devices
        // use the same address (0x68)
         I2CReadRegister(i, 0x12, cTemp, 1); // read temp reg
         if ((cTemp[0] & 0x3f) == 0) {
            iType = SENSOR_DS3231;
         }
    }
    if (iType == SENSOR_UNKNOWN && i == 0x51) {
       // The PCF8563 and RV3032 use the same I2C address (0x51)
       // Try to write to the temperature threshold register to see
       // which one is connected
       cTemp[0] = 0x17; // temp threshold high register
       cTemp[1] = 0x55; // random value to write
       I2CWrite(i, cTemp, 2);
       I2CReadRegister(i, 0x17, cTemp, 1);
       if (cTemp[0] == 0x55) {
          iType = SENSOR_RV3032;
       } // else it must be the PCF8563
    }
    if (iType == SENSOR_UNKNOWN) {
    // Check for LSM6DS3
    	I2CReadRegister(i, 0x0f, cTemp, 1); // WHO_AM_I
    	if (cTemp[0] == 0x69) {
    		iType = SENSOR_LSM6DS3;
    	}
    }

    strcpy(szName, szSensorNames[iType]);
	return iType;
} /* GetSensorType() */

void ScanBus(void)
{
uint8_t i, y;
int iBad = 0;

I2CInit(SDA_PIN, SCL_PIN, 100000);
scan_again:
	sharpFill(0);
    	sharpWriteString(4, 12, "I2C Bus Scan", FONT_12x16, 0);
	sharpWriteBuffer();
	while (GetButtons() != 0) {
//		Delay_Ms(100);
	}
	y = 28;
	for (i=4; i<128 && iSensor == SENSOR_UNKNOWN && iBad < 10; i++) {
		digitalWrite(LED_PIN, i & 1);
	    sharpWriteString(2, y, "0x", FONT_8x8, 0);
	    i2hex(szTemp, i);
	    sharpWriteString(-1, -1, szTemp, FONT_8x8, 0);
		sharpWriteBuffer();
		if (I2CTest(i)) {
			if (i < 16) {
				iBad++; // there shouldn't be anything with an address of less than 16
				continue;
			} // bad address
		    sharpWriteString(2, y, "0x", FONT_8x8, 0);
		    i2hex(szTemp, i);
		    sharpWriteString(-1, -1, szTemp, FONT_8x8, 0);
		    sharpWriteString(-1, -1, " --> ", FONT_8x8, 0);
		    iSensor = GetSensorType(i, szTemp);
		    sharpWriteString(-1, -1, szTemp, FONT_8x8, 0);
			sharpWriteBuffer();
			y+= 8;
		}
	}
	if (iSensor == SENSOR_UNKNOWN) {
		if (iBad >= 10) {
			sharpWriteString(2, y, "bus open error", FONT_8x8, 0);
		} else {
			sharpWriteString(2, y, "No sensors found", FONT_8x8, 0);
		}
		y += 8;
		sharpWriteString(2, y, "Press button to scan again", FONT_6x8, 0);
		sharpWriteBuffer();
		while (GetButtons() == 0) {
			//Delay_Ms(25);
		}
		goto scan_again;
	}
	i = 0;
	sharpWriteString(118, 2, "Start", FONT_8x8, 0);
	sharpWriteString(110, 58, "Invert", FONT_8x8, 0);
	while (i == 0) { // wait for user to start or invert the colors
		sharpWriteBuffer();
		while (GetButtons() == 0) {
			//Delay_Ms(25);
		}
		if (GetButtons() == 1) {
			while (GetButtons() != 0) {}; // wait for the user to release the button
			i = 1; // break out of loop and start sensing
		}
		if (GetButtons() == 2) {
			bInvert = ~bInvert;
			sharpInvert();
			sharpWriteBuffer();
			while (GetButtons() != 0) {
				// wait for button release
				//Delay_Ms(25);
			}
		}
	} // while waiting for user to start
} /* ScanBus() */

void TIM2_PWMOut_Init(u16 arr, u16 psc, u16 ccp)
{
#ifdef FUTURE
    GPIO_InitTypeDef GPIO_InitStructure={0};
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD , ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 << (LCD_VCOM & 0xf);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOD, &GPIO_InitStructure );

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init( TIM2, &TIM_OCInitStructure );

    TIM_CtrlPWMOutputs(TIM2, ENABLE );
    TIM_OC2PreloadConfig( TIM2, TIM_OCPreload_Disable );
    TIM_ARRPreloadConfig( TIM2, ENABLE );
    TIM_Cmd( TIM2, ENABLE );
#endif // FUTURE
} /* TIM2_PWMOut_Init() */

void ShowBME280(uint32_t T, uint32_t P, uint32_t H)
{

     sharpFill(bInvert);
     sharpWriteString(2,2, "Temp: ", FONT_12x16, bInvert);
     i2str(szTemp, udiv32(T,100));
     szTemp[2] = '.';
     i2str(&szTemp[3], umod32(T, 100));
     szTemp[5] = 'C';
     szTemp[6] = 0;
     sharpWriteString(-1, -1, szTemp, FONT_12x16, bInvert);

     sharpWriteString(2,22, "Hum: ", FONT_12x16, bInvert);
     i2str(szTemp, (int)(H>>10));
     sharpWriteString(-1, -1, szTemp, FONT_12x16, bInvert);
     sharpWriteString(-1, -1, "%", FONT_12x16, bInvert);

     sharpWriteString(2,42, "Pres: ", FONT_12x16, bInvert);
     i2str(szTemp, (int)P);
     sharpWriteString(-1, -1, szTemp, FONT_12x16, bInvert);
     sharpWriteBuffer();
} /* ShowBME280() */

void RunBME280(void)
{
uint32_t T, P, H;

	bme280Init();
	while (1) {
		bme280ReadValues(&T, &P, &H);
		ShowBME280(T, P, H);
		Delay_Ms(1000);
	}
} /* RunBME280() */

void RunLTR390(void)
{
	int i, iMax, iHead = 0;
	int iSamples[32]; // keep about 3 seconds for the max value

	memset(iSamples, 0, sizeof(iSamples));
	ltr390_init(SDA_PIN, SCL_PIN, 400000);
	ltr390_start(1); // start UV sensor
    while(1)
    {
    //	digitalWrite(LED_PIN, 1);
    	ltr390_getSample();
    	iSamples[iHead++] = _iUV;
    	iHead &= 0x1f; // circular buffer
    	Delay_Ms(50); // default sample rate = 100ms
    //	digitalWrite(LED_PIN, 0);
//    	Delay_Ms(450);
    	// find the max value of the samples collected
    	iMax = 0;
    	for (i=0; i<32; i++) {
    		if (iSamples[i] > iMax) iMax = iSamples[i];
    	}
    	ShowLTR390Sample(_iUV, iMax);
    }

} /* RunLTR390() */

void ShowCO2(void)
{
	int i, x;

			sharpFill(bInvert);
			sharpWriteString(132,2, "CAL", FONT_8x8, bInvert);
	        i = i2str(szTemp, (int)_iCO2);
	        sharpWriteStringCustom(&Roboto_Black_40, 0, 32, szTemp, !bInvert, 1);
	        x = sharpGetCursorX();
	        if (i < 4) {
	           sharpWriteString(x+24, 0, "  ", FONT_12x16, bInvert); // make sure old data is erased if going from 4 to 3 digits
	           sharpWriteString(x, 16, "   ", FONT_12x16, bInvert);
	        }
	        sharpWriteString(x, 2, "CO2", FONT_8x8, bInvert);
	        sharpWriteString(x, 10, "ppm", FONT_8x8, bInvert);

	        sharpWriteString(2, 36, "Temp ", FONT_12x16, bInvert);
	        i2str(szTemp, udiv32(_iTemperature,10)); // whole part
	        sharpWriteString(-1, 36, szTemp, FONT_12x16, bInvert);
	        i2str(szTemp, umod32(_iTemperature, 10)); // fraction
	        sharpWriteString(-1, 36, ".", FONT_12x16, bInvert);
	        sharpWriteString(-1, 36, szTemp, FONT_12x16, bInvert);
	        sharpWriteString(-1, 36, "C", FONT_12x16, bInvert);

	        sharpWriteString(2, 52, "Humidity ", FONT_12x16, bInvert);
	        i2str(szTemp, udiv32(_iHumidity,10)); // throw away fraction since it's not accurate
	        sharpWriteString(-1, 52, szTemp, FONT_12x16, bInvert);
	        sharpWriteString(-1, 52, "%", FONT_12x16, bInvert);
	        sharpWriteBuffer();
} /* ShowCO2() */

#ifdef USE_IMU
void ShowIMU(void)
{
	int16_t acc[3];

	IMUGetSample(acc, NULL, NULL); // get accelerometer samples
	sharpFill(0);
	sharpWriteString(2, 4, "X: ", FONT_12x16, 0);
	i2str(szTemp, acc[0]);
	sharpWriteString(-1, -1, szTemp, FONT_12x16, 0);

	sharpWriteString(2, 24, "Y: ", FONT_12x16, 0);
	i2str(szTemp, acc[1]);
	sharpWriteString(-1, -1, szTemp, FONT_12x16, 0);

	sharpWriteString(2, 44, "Z: ", FONT_12x16, 0);
	i2str(szTemp, acc[2]);
	sharpWriteString(-1, -1, szTemp, FONT_12x16, 0);
	sharpWriteBuffer();
} /* ShowIMU() */
#endif // USE_IMU

void ShowCountdown(int iSecs)
{
    szTemp[0] = '0';
	szTemp[1] = udiv32(iSecs,60)+'0';
	szTemp[2] = ':';
	szTemp[3] = udiv32(umod32(iSecs, 60), 10) + '0';
	szTemp[4] = umod32(iSecs, 10) + '0';
	szTemp[5] = 0;
	sharpWriteStringCustom(&Roboto_Black_40, 10, 56, szTemp, 1-bInvert, 1);
	sharpWriteBuffer();
} /* ShowCountdown() */

void CO2Calibrate(void)
{
	int i, j;

	sharpFill(bInvert);
	sharpWriteString(2,2,"Calibrating..", FONT_12x16, bInvert);
	sharpWriteBuffer();
	while (GetButtons() != 0) {};
//   scd41_start(SCD_POWERMODE_NORMAL);
   // allow 3 minutes of normal collection
   for (i=210; i>=0; i--) {
	  ShowCountdown(i);
	  j = GetButtons();
	  if (j == 3) { // user quit
		  scd41_stop();
		  return;
	  }
	  Delay_Ms(1000);
   }
   scd41_stop(); // stop periodic measurement
   i = scd41_recalibrate(423); // force recalibration
   if (i == SCD_SUCCESS)
	   sharpWriteString(2,32, "Success!", FONT_12x16, bInvert);
   else
	   sharpWriteString(2,32, "Failed", FONT_12x16, bInvert);
   sharpWriteBuffer();
//   sharpWriteString(2,56, "Press button to exit", FONT_8x8, 0);
   while (GetButtons() == 0) {
//	   Delay_Ms(20);
   }

} /* CO2Calibrate() */

void RunSCD4X(void)
{
    I2CInit(SDA_PIN, SCL_PIN, 100000);
    scd41_start(SCD_POWERMODE_NORMAL);
    while (1) {
    	if (GetButtons() == 3) {
    		CO2Calibrate();
    	}
        scd41_getSample();
        ShowCO2();
        Delay_Ms(5000); // 5 seconds per sample
    }
} /* RunSCD4X() */

#ifdef USE_RTC
void RunRTC(void)
{
	int iTick = 0;
	rtcInit((iSensor == SENSOR_DS3231) ? RTC_DS3231 : RTC_RV3032, SDA_PIN, SCL_PIN);
	rtcGetTime(&myTime);
	while (1) {
		if (GetButtons() == 1) {
			while (GetButtons() != 0) {};
			SetTime();
		} else if (GetButtons() == 2) {
			bInvert = ~bInvert;
			sharpInvert();
			sharpWriteBuffer();
			while (GetButtons() != 0) {};
		}
		Delay_Ms(33);
		if ((iTick & 0x1f) == 0) {
			ShowTime();
		}
		iTick++;
	}
} /* RunRTC() */
#endif // USE_RTC

#ifdef USE_IMU
void RunIMU(void)
{
	IMUStart(200, 0, 0); // start accelerometer at 200 samples/sec
	while (1) {
		ShowIMU(); // run as fast as possible
	}
} /* RunIMU() */
#endif

#ifdef FUTURE
void ShowLogo(void)
{
ISBICIMG img;
int x, y, j;
uint8_t uc, *d;

	if (isbicDecodeInit(&img, (uint8_t *)psp_logo)) {
		for (y=0; y<LCD_HEIGHT; y++) {
			d = sharpGetBuffer();
			d += (y * LCD_PITCH);
			for (x=0; x<LCD_WIDTH; x+=8) {
				for (j=0; j<8; j++) {
					uc <<= 1;
					if (!isbicDecode1Pixel(&img)) {
						uc |= 1;
					}
				} // for j
				*d++ = uc;
			} // for x
		} // for y
	}
} /* ShowLogo() */
#endif // FUTURE

int main(void)
{
int i;
    SystemInit();
    TIM2_PWMOut_Init( 20, 65535, 10 ); // start a 50% duty cycle PWM output on D3 of about 4hz
    pinMode(BUTTON0_PIN, INPUT_PULLUP);
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    sharpInit(8000000, LCD_CS);
    sharpFill(0);
    sharpWriteBuffer();
//    ShowLogo(); // decode the logo image
//    sharpWriteBuffer();
//    Delay_Ms(3000);
//    sharpFill(0);
    sharpWriteString(6, 2, "CH32V003", FONT_12x16, 0);
    sharpWriteString(18, 18, "Sensor", FONT_12x16, 0);
    sharpWriteString(6, 34, "Platform", FONT_12x16, 0);
    sharpWriteString(2, 50, "by Larry Bank", FONT_8x8, 0);
    sharpWriteString(136, 2,"GPS", FONT_8x8, 0);
    sharpWriteString(136, 58, "I2C", FONT_8x8, 0);
    sharpWriteBuffer();
    Delay_Ms(2000);
//    pinMode(LCD_VCOM, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    i = 0;
    while (GetButtons() == 0) {
    	digitalWrite(LED_PIN, i & 1);
    	Delay_Ms(250);
    	i++;
    }
    i = GetButtons();
    digitalWrite(LED_PIN, 0);
    while (GetButtons() != 0) {
    	//Delay_Ms(100);
    }
    if (i == 1) { // GPS option
       RunGPS();
    } else { // I2C option
       ScanBus(); // if we return from here, we have a recognized sensor
       switch (iSensor) { // start displaying sensor data
#ifdef USE_IMU
    	case SENSOR_LSM6DS3:
    		RunIMU();
    		break;
#endif // USE_IMU
#ifdef USE_BME
       case SENSOR_BME280:
                RunBME280();
                break;
#endif
#ifdef USE_RTC
    	case SENSOR_DS3231:
    	case SENSOR_RV3032:
    		RunRTC();
    		break;
#endif // USE_RTC
    	case SENSOR_LTR390:
    		RunLTR390();
    		break;
    	case SENSOR_SCD4X:
    		RunSCD4X();
    		break;
       } // switch
   } // I2C option
} /* main() */
