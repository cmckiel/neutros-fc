# Project: Neutros FC
cmake_minimum_required(VERSION 3.20)
project(neutros-fc C)

# Include HAL in the project.
add_subdirectory(lib/hal)
add_library(Hal::stm32f4_hal ALIAS stm32f4_hal)

add_subdirectory(src)

# Define the application main and link to HAL.
add_executable(neutros_fc ${CMAKE_SOURCE_DIR}/src/fc_main/main.c)
target_link_libraries(neutros_fc PRIVATE Hal::stm32f4_hal scheduler)

# Strip the elf and prepare a bin to flash
add_custom_target(neutros_fc_binary ALL
    arm-none-eabi-objcopy -O binary neutros_fc neutros_fc.bin
    DEPENDS neutros_fc
)
