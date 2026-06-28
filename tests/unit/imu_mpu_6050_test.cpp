#include "gtest/gtest.h"

extern "C" {
  #include "imu.h"
}

class MPU6050_DriverTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

/**
 * @brief Verifies the IMU initializes correctly.
 *
 * @verifies LLR_IMU_001
 */
TEST_F(MPU6050_DriverTest, InitsCorrectly)
{
  EXPECT_TRUE(imu_init());
}
