add_library(
  motor STATIC
  src/motor.c
  src/pwm_motor.c
)

target_include_directories(
  motor
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_SOURCE_DIR}/src/comms/c2/include
)

target_link_libraries(
  motor
  PRIVATE
  Hal::stm32f4_hal
  blackboard
)

