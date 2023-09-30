//
// STMicro LSM6DS3 IMU library
// written by Larry Bank 7/1/2023
//
#include "LSM6DS3.h"

const int16_t lsm6ds3_rates[] = {0, 12, 26, 52, 104, 208, 416, 833, 1660, 3330, 6660, -1};

//
// Read a 16-bit signed integer value from 2 bytes stored at the given pointer addr
// The member variable _bBigEndian determines the byte order
//
int16_t get16Bits(uint8_t *s)
{
int16_t i;

#ifdef BIG_ENDIAN
       i = (int16_t)s[0] << 8;
       i |= s[1];
#else
       i = (int16_t)s[1] << 8;
       i |= s[0];
#endif
   return i;
} /* get16Bits() */

//
// Match the requested rate to the closest supported value
//
int matchRate(int value, int16_t *pList)
{
int index = 0;

   while (pList[index] != -1) {
     if (value > pList[index] && value <= pList[index+1]) {
         break;
     }
     index++;
   }
   if (pList[index] == -1)
      index--; // return the max value

   return index;
} /* matchRate() */

void IMUStart(int iAccelRate, int iGyroRate, int bStep)
{
int iRate;
uint8_t ucTemp[4];

  if (iAccelRate) {
    iRate = 1 + matchRate(iAccelRate, (int16_t *)lsm6ds3_rates);
    iAccelRate = lsm6ds3_rates[iRate]; // get the quantized value
    ucTemp[0] = 0x10; // CTRL1_XL
    ucTemp[1] = (iRate<<4); // iODR << 4;
    I2CWrite(IMU_ADDR, ucTemp, 2);
    ucTemp[0] = 0x15; // CTR6_C - accel power mode
    if (iAccelRate <= 52)
       ucTemp[1] = 0x10; // disable high perf mode
    else
       ucTemp[1] = 0x00; // enable high perf mode, enable high pass filter
    I2CWrite(IMU_ADDR, ucTemp, 2);
  } // start accelerometer

  if (iGyroRate) {
      iRate = 1 + matchRate(iGyroRate, (int16_t *)lsm6ds3_rates);
      ucTemp[0] = 0x11; // CTRL2_G
      if (iRate > 8) iRate = 8; // Gyro max rate = 1660hz
      iGyroRate = lsm6ds3_rates[iRate]; // get the quantized value
      ucTemp[1] = (iRate<<4); // gyroscope data rate
      I2CWrite(IMU_ADDR, ucTemp, 2);
      ucTemp[0] = 0x16; // CTR7_G - gyro power mode
      if (iGyroRate <= 52)
         ucTemp[1] = 0x80; // Enable low power mode
      else
         ucTemp[1] = 0x40; // Disable low power mode, enable high pass filter
      I2CWrite(IMU_ADDR, ucTemp, 2);
  } // start gyroscope

  if (bStep) {
      ucTemp[0] = 0x19; // CTRL10_C
      ucTemp[1] = (iGyroRate > 0) ? 0x3f : 0x7; // check 3 axis of gyro are enabled (on by default)
      I2CWrite(IMU_ADDR, ucTemp, 2);
      ucTemp[0] = 0x58; // TAP_CFG
      ucTemp[1] = 0x4f; // enable step counter + tap detection on x/y/z + latch interrupt
      I2CWrite(IMU_ADDR, ucTemp, 2);
//      ucTemp[0] = 0x13; // SM_THS - significant motion threshold
//      ucTemp[1] = 0x2; // very sensitive
//      I2CWrite(IMU_ADDR, ucTemp, 2);
      ucTemp[0] = 0xd; // INT1_CTRL
      ucTemp[1] = 0x40; // signficant motion detection interrupt
      I2CWrite(IMU_ADDR, ucTemp, 2);
      ucTemp[0] = 0x5e; // MD1_CFG - routing of INT1 events
      ucTemp[1] = 0x60; // single tap + wake-up
      I2CWrite(IMU_ADDR, ucTemp, 2);
  } // start step counter

} /* IMUStart() */

void IMUStop(void)
{

} /* IMUStop() */

void IMUGetSample(int16_t *pAcc, int16_t *pGyro, int16_t *pSteps)
{
uint8_t ucTemp[8];
int i;

	if (pAcc) { // get accelerometer samples
        I2CReadRegister(IMU_ADDR, IMU_ACC_START, ucTemp, 6);
        for (i=0; i<3; i++) {
           pAcc[i] = get16Bits(&ucTemp[i*2]);
        }
	}
	if (pGyro) {
	    I2CReadRegister(IMU_ADDR, IMU_GYRO_START, ucTemp, 6);
	    for (i=0; i<3; i++) {
	       pGyro[i] = get16Bits(&ucTemp[i*2]);
	    }
	}
	if (pSteps) {
        I2CReadRegister(IMU_ADDR, IMU_STEP_START, ucTemp, 2);
        *pSteps = get16Bits(ucTemp);
	}
} /* IMUGetSample() */
