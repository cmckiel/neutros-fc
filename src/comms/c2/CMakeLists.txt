add_library(
  c2 STATIC
  src/c2.c
)

target_include_directories(
  c2
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  c2
  PRIVATE
  Hal::stm32f4_hal
  config
  blackboard
)
