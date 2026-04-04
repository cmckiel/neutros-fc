add_library(
  services STATIC
  src/i2c_servicer.c
)

target_include_directories(
  services
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  services
  PRIVATE
  Hal::stm32f4_hal
)
