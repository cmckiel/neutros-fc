#ifndef _IMU_TYPES_H
#define _IMU_TYPES_H

#include <stdint.h>

typedef struct {
  float x_angular_velocity_dps;
  float y_angular_velocity_dps;
  float z_angular_velocity_dps;
} imu_gyro_t;

typedef struct {

} imu_blackboard_private_data_t;

typedef struct {
  imu_gyro_t gyroscope;
} imu_blackboard_public_data_t;

typedef struct {
  imu_blackboard_private_data_t private_data;
  imu_blackboard_public_data_t public_data;
} imu_blackboard_data_t;

#endif /* _IMU_TYPES_H */

