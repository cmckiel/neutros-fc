add_library(
  imu STATIC
  src/imu.c
  src/imu_mpu6050.c
)

target_include_directories(
  imu
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  imu
  PRIVATE
  Hal::stm32f4_hal
  blackboard
)

