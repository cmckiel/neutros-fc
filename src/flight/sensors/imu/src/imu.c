#include "imu.h"
#include "imu_types.h"
#include "imu_mpu6050.h"
#include "blackboard.h"
#include "blackboard_topic_ids.h"

static imu_blackboard_data_t *imu_blackboard_data;

bool imu_init()
{
  imu_blackboard_data = blackboard_get_publisher_handle(IMU_TOPIC_ID);
  return imu_mpu6050_init();
}

bool imu_exec()
{
  float gx_dps = 0;
  float gy_dps = 0;
  float gz_dps = 0;

  bool res = imu_mpu6050_get_angular_acceleration(&gx_dps, &gy_dps, &gz_dps);

  if (res)
  {
    imu_blackboard_data->public_data.gyroscope.x_angular_velocity_dps = gx_dps;
    imu_blackboard_data->public_data.gyroscope.y_angular_velocity_dps = gy_dps;
    imu_blackboard_data->public_data.gyroscope.z_angular_velocity_dps = gz_dps;
  }

  return false;
}
