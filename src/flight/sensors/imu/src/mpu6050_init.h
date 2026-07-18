#ifndef _IMU_MPU6050_INIT_H
#define _IMU_MPU6050_INIT_H

#include <stdbool.h>
#include "mpu6050.h"

#define MPU_OFFSET_AVERAGE_SAMPLE_SIZE 500

bool mpu6050_whoami();

bool mpu6050_set_clock_source_gyro_x();

bool mpu6050_perform_self_test(float *percent_diff_ft_xg, float *percent_diff_ft_yg, float *percent_diff_ft_zg,
                               float *percent_diff_ft_xa, float *percent_diff_ft_ya, float *percent_diff_ft_za);

bool mpu6050_set_fs_sel_gyro(mpu6050_fs_sel_gyro_t fs_select);

bool mpu6050_calc_gyro_offsets(int16_t *xg_offset, int16_t *yg_offset, int16_t *zg_offset);

bool mpu6050_calc_accel_offsets(float *xa_offset, float *ya_offset, float *za_offset);

#endif /* _IMU_MPU6050_INIT_H */
