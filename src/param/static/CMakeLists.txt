add_library(
  config STATIC
  src/config.c
)

target_include_directories(
  config
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Probably don't need to link against full HAL. We just need the data types.
# Could decide to include those directories instead.
target_link_libraries(
  config
  PRIVATE
  Hal::stm32f4_hal
)
