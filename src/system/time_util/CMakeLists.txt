add_library(
  time_util STATIC
  src/time_util.c
)

target_include_directories(
  time_util
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  time_util
  PRIVATE
  Hal::stm32f4_hal
)
