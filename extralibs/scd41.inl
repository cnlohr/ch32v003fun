/*
 * scd41.c
 *
 *  Created on: Jan 10, 2023
 *      Author: larry
 */
#include "scd41.h"

static int _iPowerMode, _iTemperature, _iHumidity;
static uint16_t _iCO2;

int scd41_getSample(void)
{
uint8_t ucTemp[16];
uint16_t u16Status;
int rc;

    if (_iPowerMode == SCD_POWERMODE_ONESHOT) {
        scd41_sendCMD(SCD41_CMD_SINGLE_SHOT_MEASUREMENT);
        Delay_Ms(5000); // wait for measurement to occur
    }
    rc = scd41_readRegister(SCD41_CMD_GET_DATA_READY_STATUS, &u16Status);
//Serial.print("status = 0x"); Serial.println(u16Status, HEX);
    if (rc != SCD_SUCCESS)
	    return rc;

    if ((u16Status & 0x07ff) == 0x0000) { // lower 11 bits == 0 -> data not ready
  //     Serial.println("data not ready!");
       return SCD_NOT_READY;
    }
    scd41_sendCMD(SCD41_CMD_READ_MEASUREMENT);
    Delay_Ms(5);
    if (I2CRead(0x62, ucTemp, 9)) { // 9 bytes of data for the 3 fields
//Serial.println("Got 9 bytes from sensor");
    _iCO2 = ((uint16_t)ucTemp[0] << 8) | ucTemp[1];
    _iTemperature = (ucTemp[3] << 8) | ucTemp[4];
    _iHumidity = ((uint16_t)ucTemp[6] << 8) | ucTemp[7];
    _iTemperature = -450 + (((_iTemperature) * 1750L) >> 16);
    _iHumidity = (_iHumidity * 1000L) >> 16;
    return SCD_SUCCESS;
    }
  return SCD_ERROR;
} /* scd41_getSample() */

void scd41_wakeup(void)
{
    scd41_sendCMD(SCD41_CMD_WAKEUP);
    Delay_Ms(20);
} /* scd41_wakeup() */

int scd41_stop(void)
{
    if (scd41_sendCMD(SCD41_CMD_STOP_PERIODIC_MEASUREMENT)) {
        Delay_Ms(500); // wait for it to execute
        return SCD_SUCCESS;
    }
    return SCD_ERROR;
} /* scd41_stop() */

int scd41_start(int iPowerMode)
{
// Start correct mode
     scd41_wakeup();
     _iPowerMode = iPowerMode;
     scd41_sendCMD2(SCD41_CMD_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED, 0); // turn off self-calibration
     Delay_Ms(5);
     if (iPowerMode == SCD_POWERMODE_NORMAL)
        scd41_sendCMD(SCD41_CMD_START_PERIODIC_MEASUREMENT);
     else if (iPowerMode == SCD_POWERMODE_LOW)
        scd41_sendCMD(SCD41_CMD_START_LP_PERIODIC_MEASUREMENT);
     else // single shot is essentially "stopped"
        scd41_sendCMD(SCD41_CMD_STOP_PERIODIC_MEASUREMENT);
     Delay_Ms(1);
     return SCD_SUCCESS;
} /* scd41_start() */

int scd41_readRegister(uint16_t u16Register, uint16_t *pOut)
{
uint8_t ucTemp[4];
//int rc;

   ucTemp[0] = (uint8_t)(u16Register >> 8);
   ucTemp[1] = (uint8_t)(u16Register);
   I2CWrite(0x62, ucTemp, 2);
 //  if (rc == 0) // something went wrong
//	   return SCD_ERROR;
   Delay_Ms(5);
   I2CRead(0x62, ucTemp, 3); // ignore CRC for now
   //if (rc == 0) // problem
//	   return SCD_ERROR;
   *pOut = (uint16_t)ucTemp[0] << 8 | ucTemp[1];
   return SCD_SUCCESS;

} /* scd41_readRegister() */

int scd41_sendCMD(uint16_t u16Cmd)
{
uint8_t ucTemp[4];

   ucTemp[0] = (uint8_t)(u16Cmd >> 8);
   ucTemp[1] = (uint8_t)(u16Cmd);
   I2CWrite(0x62, ucTemp, 2);
   return SCD_SUCCESS;
} /* scd41_sendCMD() */

int scd41_sendCMD2(uint16_t u16Cmd, uint16_t u16Parameter)
{
uint8_t ucTemp[8];

   ucTemp[0] = (uint8_t)(u16Cmd >> 8);
   ucTemp[1] = (uint8_t)(u16Cmd);
   ucTemp[2] = (uint8_t)(u16Parameter >> 8);
   ucTemp[3] = (uint8_t)(u16Parameter);
   ucTemp[4] = scd41_computeCRC8(&ucTemp[2], 2); // CRC for arguments only
   I2CWrite(0x62, ucTemp, 5);
   return SCD_SUCCESS;

} /* scd41_sendCMD2() */
//Given an array and a number of bytes, this calculate CRC8 for those bytes
//CRC is only calc'd on the data portion (two bytes) of the four bytes being sent
//From: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
//Tested with: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
//x^8+x^5+x^4+1 = 0x31
uint8_t scd41_computeCRC8(uint8_t *data, uint8_t len)
{
  uint8_t crc = 0xFF; //Init with 0xFF

  for (uint8_t x = 0; x < len; x++)
  {
    crc ^= data[x]; // XOR-in the next input byte

    for (uint8_t i = 0; i < 8; i++)
    {
      if ((crc & 0x80) != 0)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }

  return crc; //No output reflection
} /* scd41_computeCRC8() */

int scd41_shutdown(void)
{
    if (scd41_sendCMD(SCD41_CMD_POWERDOWN)) {
        Delay_Ms(1);
        return SCD_SUCCESS;
    }
    return SCD_ERROR;
} /* scd41_shutdown() */

void scd41_setAutoCalibration(int bOn)
{
	scd41_sendCMD2(SCD41_CMD_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED, bOn);
} /* scd41_setAutoCalibration() */

int scd41_recalibrate(uint16_t u16CO2)
{
uint8_t ucTemp[4];

	scd41_sendCMD2(SCD41_CMD_FORCE_RECALIBRATE, u16CO2); // set the reference CO2 level
	Delay_Ms(400); // wait to complete
    I2CRead(0x62, ucTemp, 3); // 3 byte response. 0xFFFF = failed
    if (ucTemp[0] == 0xff && ucTemp[1] == 0xff)
    	return SCD_ERROR;
    else
    	return SCD_SUCCESS;
} /* scd41_recalibrate() */



