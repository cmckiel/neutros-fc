add_library(
  blackboard STATIC
  src/blackboard.c
  src/blackboard_topics.c
)

target_include_directories(
  blackboard
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_SOURCE_DIR}/src/comms/c2/include
)
