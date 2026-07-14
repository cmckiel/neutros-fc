#ifndef _IMU_MPU6050_INIT_H
#define _IMU_MPU6050_INIT_H

#include <stdbool.h>

bool mpu6050_whoami();

bool mpu6050_set_clock_source_gyro_x();

bool mpu6050_perform_self_test(float *percent_diff_ft_xg, float *percent_diff_ft_yg, float *percent_diff_ft_zg,
                               float *percent_diff_ft_xa, float *percent_diff_ft_ya, float *percent_diff_ft_za);

#endif /* _IMU_MPU6050_INIT_H */
