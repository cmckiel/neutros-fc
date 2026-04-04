#include "blackboard_topics.h"

#include "c2_types.h"
#include "imu_types.h"

c2_blackboard_data_t c2_blackboard_data;
imu_blackboard_data_t imu_blackboard_data;

blackboard_topic_t blackboard_topics[_BLACKBOARD_TOPIC_ID_MAX] = {
  [C2_TOPIC_ID] = { .name = "c2_topic", .topic_address = &c2_blackboard_data, .topic_is_published = false },
  [IMU_TOPIC_ID] = { .name = "imu_topic", .topic_address = &imu_blackboard_data, .topic_is_published = false},
};
