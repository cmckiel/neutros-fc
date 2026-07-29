#ifndef _IMU_MPU6050_H
#define _IMU_MPU6050_h

#include <stdbool.h>

#include "mpu6050.h"

bool imu_mpu6050_init();
bool imu_mpu6050_get_imu_data(float *xa, float *ya, float *za, float *xg, float *yg, float *zg);

#endif /* _IMU_MPU6050_H */

