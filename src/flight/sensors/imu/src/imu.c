#include "imu.h"
#include "imu_types.h"
#include "imu_mpu6050.h"
#include "blackboard.h"
#include "blackboard_topic_ids.h"
#include "mpu6050.h"

static imu_blackboard_data_t *imu_blackboard_data;

bool imu_init()
{
  imu_blackboard_data = blackboard_get_publisher_handle(IMU_TOPIC_ID);
  return imu_mpu6050_init();
}

bool imu_exec()
{
  float xa;
  float ya;
  float za;

  float xg;
  float yg;
  float zg;

  bool res = imu_mpu6050_get_imu_data(&xa, &ya, &za, &xg, &yg, &zg);

  if (res)
  {
    imu_blackboard_data->public_data.gyroscope.x_angular_velocity_dps = xg;
    imu_blackboard_data->public_data.gyroscope.y_angular_velocity_dps = yg;
    imu_blackboard_data->public_data.gyroscope.z_angular_velocity_dps = zg;

    imu_blackboard_data->public_data.accelerometer.x_accel = xa;
    imu_blackboard_data->public_data.accelerometer.y_accel = ya;
    imu_blackboard_data->public_data.accelerometer.z_accel = za;
  }

  return false;
}
