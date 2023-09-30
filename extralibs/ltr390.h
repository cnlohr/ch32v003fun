//
// LTR390 - Lite-On Visible/UV light sensor library
//
// Written by Larry Bank - 6/6/2022
// Copyright (c) 2022 BitBank Software, Inc.
// bitbank@pobox.com
//
//
#ifndef __LTR390__
#define __LTR390__

#include <stdint.h>
#include <string.h>

// This selects floating point functions for the UVI and Lux values
// Some micros (e.g. CH32V) don't have hardware floating point, so using floats
// adds about 1K of library code and slows things down
//#define USE_FLOATS

#define LTR390_SUCCESS 0
#define LTR390_ERROR -1
#define LTR390_BAD_PARAMETER -2


// Register definitions
#define LTR390_MAIN_CTRL 0x00
#define LTR390_MEAS_RATE 0x04
#define LTR390_ALS_UVS_GAIN 0x05
#define LTR390_MAIN_STATUS 0x07
#define LTR390_ALS_DATA_0 0x0D
#define LTR390_UVS_DATA_0 0x10

int ltr390_init(uint8_t sda, uint8_t scl, int32_t iSpeed);
int ltr390_start(int bUV);
int ltr390_stop(void);
int ltr390_reset(void);
uint8_t ltr390_status(void);
void ltr390_getSample(void); // trigger + read the latest data
int ltr390_setResolution(int iRes);
int ltr390_setGain(int iGain);
int ltr390_visible(void);
int ltr390_uv(void);
#ifdef USE_FLOATS
float ltr390_getLux(int);
float ltr390_getUVI(int);
#else
int ltr390_getLux(int);
int ltr390_getUVI(int);
#endif // USE_FLOATS

#endif // __LTR390__
