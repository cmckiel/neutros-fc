add_library(
  scheduler STATIC
  src/scheduler.c
)

target_include_directories(
  scheduler
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  scheduler
  PRIVATE
  Hal::stm32f4_hal
  c2
  telemetry
)
