#ifndef _IMU_MPU6050_H
#define _IMU_MPU6050_h

#include <stdbool.h>

bool imu_mpu6050_init();
bool imu_mpu6050_get_angular_acceleration(float *gx_dps_ptr, float *gy_dps_ptr, float *gz_dps_ptr);

#endif /* _IMU_MPU6050_H */

