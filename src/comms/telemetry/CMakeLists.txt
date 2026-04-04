add_library(
  telemetry STATIC
  src/telemetry.c
)

target_include_directories(
  telemetry
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_SOURCE_DIR}/src/comms/c2/include
  ${CMAKE_SOURCE_DIR}/src/flight/sensors/imu/include
)

target_link_libraries(
  telemetry
  PRIVATE
  Hal::stm32f4_hal
  config
  blackboard
)
