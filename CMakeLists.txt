# Project: Neutros FC
cmake_minimum_required(VERSION 3.20)
project(neutros-fc C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
include(Doxygen)
include(GenerateTrace)
include(CheckTrace)
include(Sphinx)

add_doxygen_target()
add_generate_trace_target()
add_check_trace_target()
add_sphinx_target()

add_subdirectory(src)

if (TARGET_BUILD)
  # Include HAL in the project.
  add_subdirectory(lib/hal)
  add_library(Hal::stm32f4_hal ALIAS stm32f4_hal)

  # Define the application main and link to HAL.
  add_executable(neutros_fc ${CMAKE_SOURCE_DIR}/src/fc_main/main.c)
  target_link_libraries(neutros_fc PRIVATE Hal::stm32f4_hal scheduler)

  # Strip the elf and prepare a bin to flash
  add_custom_target(neutros_fc_binary ALL
      arm-none-eabi-objcopy -O binary neutros_fc neutros_fc.bin
      DEPENDS neutros_fc
  )
endif()

if (DESKTOP_BUILD)
  # ------- DESKTOP TESTING -------

  # HAL headers only — no STM32 implementation.
  add_subdirectory(lib/hal/include)

  # Build the HAL mock library and alias it as the real HAL target so all
  # application libraries resolve their HAL dependency without modification.
  add_subdirectory(tests/mocks)
  add_library(Hal::stm32f4_hal ALIAS hal_mock)

  # Include CTest to allow VS Code to see unit tests.
  include(CTest)

  # Add our unit tests.
  add_subdirectory(tests/unit)

  # Pull in GTest.
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()
