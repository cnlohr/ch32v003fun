#include "bme280.h"

//
// BME280 temperature/pressure/humidity sensor
// code to initialize and read+calculate the calibrated values
// from the I2C bus
//
// Written by Larry Bank - 2/1/2017
// Copyright (c) 2017 BitBank Software, Inc.
// bitbank@pobox.com
// updated 2/13/2018 for Arduino
//
// Sensor calibration data
static uint8_t bAddr;
static int32_t calT1,calT2,calT3;
static int32_t calP1, calP2, calP3, calP4, calP5, calP6, calP7, calP8, calP9;
static int32_t calH1, calH2, calH3, calH4, calH5, calH6;

int32_t GetInt16(uint8_t *p)
{
   uint32_t u32;
   u32 = (uint32_t)p[0] | ((uint32_t)p[1] << 8);
   if (u32 > 32767L) u32 -= 65536L; // negative value
   return (int32_t)u32;
} /* GetInt16() */

//
// Opens a file system handle to the I2C device
// reads the calibration data and sets the device
// into auto sensing mode
//
int bme280Init(void)
{
uint8_t ucTemp[32];
uint8_t ucCal[36];

	if (I2CTest(0x76))
            bAddr = 0x76;
        else
            bAddr = 0x77;
   I2CReadRegister(bAddr, 0xd0, ucTemp, 1); // get ID
   if (ucTemp[0] != 0x60)
   {
     // printf("bme280 sensor ID doesn't match\n");
     return -1;
   }
      //printf("bme280 ID matches!\n");
   // Read 24 bytes of calibration data
   I2CReadRegister(bAddr, 0x88, ucCal, 24); // 24 from register 0x88
   I2CReadRegister(bAddr, 0xa1, &ucCal[24], 1); // get humidity calibration byte
   I2CReadRegister(bAddr, 0xe1, &ucCal[25], 7); // get 7 more humidity calibration bytes
        // Prepare temperature calibration data
        calT1 = (uint32_t)ucCal[0] + ((uint32_t)ucCal[1] << 8);
        calT2 = GetInt16(&ucCal[2]);
        calT3 = GetInt16(&ucCal[4]);

        // Prepare pressure calibration data
        calP1 = (uint32_t)ucCal[6] + ((uint32_t)ucCal[7] << 8);
        calP2 = GetInt16(&ucCal[8]);
        calP3 = GetInt16(&ucCal[10]);
        calP4 = GetInt16(&ucCal[12]);
        calP5 = GetInt16(&ucCal[14]);
        calP6 = GetInt16(&ucCal[16]);
        calP7 = GetInt16(&ucCal[18]);
        calP8 = GetInt16(&ucCal[20]);
        calP9 = GetInt16(&ucCal[22]);

        // Prepare humidity calibration data
        calH1 = (uint32_t)ucCal[24];
        calH2 = GetInt16(&ucCal[25]);
        calH3 = (uint32_t)ucCal[27];
        calH4 = ((uint32_t)ucCal[28] << 4) + ((uint32_t)ucCal[29] & 0xf);
        if (calH4 > 2047L) calH4 -= 4096L; // signed 12-bit
        calH5 = ((uint32_t)ucCal[30] << 4) + ((uint32_t)ucCal[29] >> 4);
        if (calH5 > 2047L) calH5 -= 4096L;
        calH6 = (uint32_t)ucCal[31];
        if (calH6 > 127L) calH6 -= 256L; // signed char

        ucTemp[0] = 0xf2;
        ucTemp[1] = 0x01; // humidity over sampling rate = 1
        I2CWrite(bAddr, ucTemp, 2); // control humidity register

        ucTemp[0] = 0xf4;
        ucTemp[1] = 0x27; // normal mode, temp and pressure over sampling rate=1
        I2CWrite(bAddr, ucTemp, 2); // control measurement register

        ucTemp[0] = 0xf5;
        ucTemp[1] = 0xa0; // set stand by time to 1 second
        I2CWrite(bAddr, ucTemp, 2); // config
 
        return 0;
} /* bme280Init() */
//
// Read the sensor register values
// and translate them into calibrated readings
// using the previously loaded calibration data
// Temperature is expressed in Celsius degrees as T * 100 (for 2 decimal places)
// Pressure is <future>
// Humidity is express as H * 1024 (10 bit fraction)
//
int bme280ReadValues(uint32_t *T, uint32_t *P, uint32_t *H)
{
uint8_t ucTemp[16];
int32_t t, p, h; // raw sensor values
int32_t var1,var2,t_fine;
int64_t P_64;
int64_t var1_64, var2_64;

        I2CReadRegister(bAddr, 0xf7, ucTemp, 8); // start of data regs
        p = ((uint32_t)ucTemp[0] << 12) + ((uint32_t)ucTemp[1] << 4) + ((uint32_t)ucTemp[2] >> 4);
        t = ((uint32_t)ucTemp[3] << 12) + ((uint32_t)ucTemp[4] << 4) + ((uint32_t)ucTemp[5] >> 4);
        h = ((uint32_t)ucTemp[6] << 8) + (uint32_t)ucTemp[7];
//      printf("raw values: p = %d, t = %d, h = %d\n", p, t, h);
        // Calculate calibrated temperature value
        // the value is 100x C (e.g. 2601 = 26.01C)
        var1 = ((((t >> 3) - (calT1 <<1))) * (calT2)) >> 11;
        var2 = (((((t >> 4) - (calT1)) * ((t>>4) - (calT1))) >> 12) * (calT3)) >> 14;
        t_fine = var1 + var2;
        t_fine = ((t_fine * 5 + 128) >> 8);
        *T = (uint32_t)(t_fine - 150L); // for some reason, the reported temp is too high, subtract 1.5C
        
        // Calculate calibrated pressure value
        var1_64 = t_fine - 128000LL;
        var2_64 = var1_64 * var1_64 * (int64_t)calP6;
        var2_64 = var2_64 + ((var1_64 * (int64_t)calP5) << 17);
        var2_64 = var2_64 + (((int64_t)calP4) << 35);
        var1_64 = ((var1_64 * var1_64 * (int64_t)calP3)>>8) + ((var1_64 * (int64_t)calP2)<<12);
        var1_64 = (((((int64_t)1)<<47)+var1_64))*((int64_t)calP1)>>33;
        if (var1_64 == 0)
        {
                *P = 0;
        }
        else
        {
                P_64 = 1048576LL - p;
                P_64 = udiv64((((P_64<<31)-var2_64)*3125LL), var1_64);
                var1_64 = (((int64_t)calP9) * (P_64>>13) * (P_64>>13)) >> 25;
                var2_64 = (((int64_t)calP8) * P_64) >> 19;
                P_64 = ((P_64 + var1_64 + var2_64) >> 8) + (((int64_t)calP7)<<4);
                *P = (uint32_t)udiv64(P_64, 100LL);
        }
        // Calculate calibrated humidity value
        var1 = (t_fine - 76800L);
        var1 = (((((h << 14) - ((calH4) << 20) - ((calH5) * var1)) +
                (16384L)) >> 15) * (((((((var1 * (calH6)) >> 10) * (((var1 * (calH3)) >> 11) + (32768L))) >> 10) + (2097152L)) * (calH2) + 8192L) >> 14));
        var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * (calH1)) >> 4));
        var1 = (var1 < 0? 0 : var1);
        var1 = (var1 > 419430400UL ? 419430400UL : var1);
        *H = (uint32_t)(var1 >> 12);
        return 0;

} /* bme280ReadValues() */
