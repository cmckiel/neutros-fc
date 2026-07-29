#ifndef _MPU6050_H
#define _MPU6050_H

#include "stdint.h"

typedef struct {
	uint8_t xg_test;
	uint8_t yg_test;
	uint8_t zg_test;

	uint8_t xa_test;
	uint8_t ya_test;
	uint8_t za_test;
} mpu6050_self_test_values_t;

typedef struct {
	int16_t xg;
	int16_t yg;
	int16_t zg;

	int16_t xa;
	int16_t ya;
	int16_t za;
} mpu6050_imu_values_t;

typedef enum {
	MPU6050_FS_SEL_GYRO_250_DPS  = 0,
	MPU6050_FS_SEL_GYRO_500_DPS  = 1,
	MPU6050_FS_SEL_GYRO_1000_DPS = 2,
	MPU6050_FS_SEL_GYRO_2000_DPS = 3,
} mpu6050_fs_sel_gyro_t;

typedef enum {
	MPU6050_FS_SEL_ACCEL_2G  = 0,
	MPU6050_FS_SEL_ACCEL_4G  = 1,
	MPU6050_FS_SEL_ACCEL_8G  = 2,
	MPU6050_FS_SEL_ACCEL_16G = 3,
} MPU6050_FS_SEL_accel_t;

// Device Address
#define MPU_6050_ADDR       0x68

// Device Registers
#define MPU_PWR_MGMT_1_REG   0x6B
#define MPU_ACCEL_XOUT_H_REG 0x3B
#define MPU_GYRO_XOUT_H_REG  0x43
#define MPU_WHOAMI_REG       0x75
#define MPU_SELF_TEST_X_REG  0x0D
#define MPU_CONFIG_REG       0x1A
#define MPU_GYRO_CONFIG_REG  0x1B
#define MPU_ACCEL_CONFIG_REG 0x1C

// Register Values
#define MPU_GYRO_WAKE        0x01
#define MPU_GYRO_SLEEP       0x40
#define MPU_GYRO_FS_SEL_2000 0x18

// Masks
#define MPU_CONFIG_DLPF_Mask         0x03
#define MPU_GYRO_CONFIG_FS_SEL_Mask  0x18
#define MPU_ACCEL_CONFIG_FS_SEL_Mask 0x18

// Shifts
#define MPU_GYRO_CONFIG_FS_SEL_Shift  3
#define MPU_ACCEL_CONFIG_FS_SEL_Shift 3

void mpu6050_unpack_self_test(uint8_t *packed_bytes, uint32_t packed_bytes_size, mpu6050_self_test_values_t *self_test_values);
void mpu6050_unpack_imu_values(uint8_t *packed_bytes, uint32_t packed_bytes_size, mpu6050_imu_values_t *imu_values);

#endif /* _MPU6050_H */
