//
// STMicro LSM6DS3 IMU library for the Pocket CO2 project
// written by Larry Bank 7/1/2023
#ifndef __LSM6DS3__
#define __LSM6DS3__

#include <stdint.h>
#include <string.h>

#define IMU_ADDR 0x6b
#define IMU_ACC_START 0x28
#define IMU_GYRO_START 0x22
#define IMU_STEP_START 0x4b

#define IMU_X_AXIS 0
#define IMU_Y_AXIS 1
#define IMU_Z_AXIS 2

void IMUStart(int iAccelRate, int iGyroRate, int bStep);
void IMUStop(void);
void IMUGetSample(int16_t *pAcc, int16_t *pGyro, int16_t *pStep);
#endif // __LSM6DS3__
