#include "ltr390.h"

//
// Written by Larry Bank - 6/6/2022
// Copyright (c) 2022 BitBank Software, Inc.
// bitbank@pobox.com
//
uint32_t _iVisible, _iUV;
static int _iGain = 3;
static int _iResolution = 18;
static int _iAddr; // I2C address of device

void ltr390_getSample(void)
{
uint8_t ucTemp[8];

    for (int i=0; i<6; i++) { // need to read them one by one
       I2CReadRegister(_iAddr, LTR390_ALS_DATA_0+i, &ucTemp[i], 1); // read ALS and UVS data together
    }
    ucTemp[2] &= 0xf; // trim to 4-bits
    ucTemp[5] &= 0xf;
    _iVisible = ucTemp[0] | (ucTemp[1] << 8) | (ucTemp[2] << 16); // 20-bits of ALS data
    _iUV = ucTemp[3] | (ucTemp[4] << 8) | (ucTemp[5] << 16); // 20-bits of UVS data
} /* ltr390_getSample() */
//
// Valid range for gain: 1,3,6,9,18
//
int ltr390_setGain(int iGain)
{
uint8_t ucTemp[4];
const uint8_t ucGain[] = {0,1,2,3,0,0,4};
   if (iGain != 1 && iGain != 3 && iGain != 6 && iGain != 9 && iGain != 18)
      return LTR390_BAD_PARAMETER;
   _iGain = iGain;
   ucTemp[0] = LTR390_ALS_UVS_GAIN;
   ucTemp[1] = ucGain[iGain];
   I2CWrite(_iAddr, ucTemp, 2);
   return LTR390_SUCCESS;
} /* ltr390_setGain() */

//
// Set the bit resolution of the samples
// valid range is 13 to 20
//
int ltr390_setResolution(int iRes)
{
uint8_t ucTemp[4];
const uint8_t ucBits[8] = {5,0,0,4,3,2,1,0};

  if (iRes != 20 && iRes != 19 && iRes != 18 && iRes != 17 && iRes != 16 && iRes != 13) return LTR390_BAD_PARAMETER;
  _iResolution = iRes;
  ucTemp[0] = LTR390_MEAS_RATE;
  ucTemp[1] = 0x2 | (ucBits[iRes-13] << 4); // default 100ms capture rate
  I2CWrite(_iAddr, ucTemp, 2);
  return LTR390_SUCCESS;  
} /* ltr390_setResolution() */

//
// Calculat the UV Index (UVI) based upon the rated sensitivity
// of 1 UVI per 2300 counts at 18X gain factor and 20-bit resolution.
//
#ifdef USE_FLOATS
float ltr390_getUVI(int i)
{
float fUVI;
float fGain;

    fUVI  = (float)(i << (20 - _iResolution)); // scale up to 20-bits of res
    fGain = (float)_iGain / 18.0f;
    fUVI  = fUVI / (fGain * 2300.0f);
    return fUVI;
} /* ltr390_getUVI() */

float ltr390_getLux(int i)
{
float lux;
const float res_factor[] = {0.03125f, 0,0,0,0.25f, 0.5f, 1.0f, 2.0f, 4.0f};

    lux = 0.6f * (float)(i) / ((float)_iGain * res_factor[_iResolution-13]);
  return lux;
} /* ltr390_getLux() */
#else // use ints on MCUs with no hardware floating point
//
// Return an integer which represents the UVI * 10.
// e.g. for a UVI of 7.8, this function will return 78
//
int ltr390_getUVI(int i)
{
uint32_t iUVI;

    iUVI  = (i << (20 - _iResolution)); // scale up to 20-bits of res
    iUVI = (iUVI * 18) / (_iGain * 230);
    return (int)iUVI;
} /* ltr390_getUVI() */

int ltr390_getLux(int i)
{
uint32_t u32, lux;
int iShift;
//const float res_factor[] = {0.03125f, 0,0,0,0.25f, 0.5f, 1.0f, 2.0f, 4.0f};
const int res_factor[] = {5, 0,0,0,2,1,0,-1,-2};

   iShift = res_factor[_iResolution-13];
   if (iShift >= 0)
      u32 = _iVisible << iShift;
   else
      u32 = _iVisible >> -iShift;
   lux = (u32 * 6) / (_iGain * 10);   
//    lux = 0.6f * (float)(_iVisible) / ((float)_iGain * res_factor[_iResolution-13]);
	return (int)lux;
} /* ltr390_getLux() */
#endif
int ltr90_reset(void)
{
uint8_t ucTemp[4];

     ucTemp[0] = LTR390_MAIN_CTRL;
     ucTemp[1] = 0x10; // assert reset
     I2CWrite(_iAddr, ucTemp, 2);
     delay(10);
     return LTR390_SUCCESS;
} /* ltr390_reset() */

int ltr390_stop(void)
{
uint8_t ucTemp[4];

     ucTemp[0] = LTR390_MAIN_CTRL;
     ucTemp[1] = 0x00; // de-activate visible or UV mode
     I2CWrite(_iAddr, ucTemp, 2);
     return LTR390_SUCCESS;
} /* ltr390_stop() */

uint8_t ltr390_status(void)
{
uint8_t ucStatus;
   I2CReadRegister(_iAddr, LTR390_MAIN_STATUS, &ucStatus, 1);
   return ucStatus;
} /* ltr390_status() */

int ltr390_start(int bUV)
{
uint8_t ucReg, ucTemp[4];

    // reset();
     I2CReadRegister(_iAddr, LTR390_MAIN_CTRL, &ucReg, 1);
     ucTemp[0] = LTR390_MAIN_CTRL;
     if (bUV)
        ucReg |= 0x08;
     else
        ucReg &= ~0x08;
     ucTemp[1] = ucReg | 0x02;
     I2CWrite(_iAddr, ucTemp, 2);
     delay(10); // allow time to start
     //setGain(3); // default
     //setResolution(18); // default
     return LTR390_SUCCESS;
} /* ltr390_start() */

int ltr390_init(uint8_t sda, uint8_t scl, int32_t iSpeed)
{
	I2CInit(sda, scl, iSpeed);
    _iAddr = 0x53;
    return LTR390_SUCCESS;
} /* ltr390_init() */

int ltr390_visible(void)
{
    return _iVisible;
} /* ltr390_visible() */

int ltr390_uv(void)
{
    return _iUV;
} /* ltr390_uv() */
