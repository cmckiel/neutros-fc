# Project: Neutros FC
cmake_minimum_required(VERSION 3.20)
project(neutros-fc C)

# Include HAL in the project.
find_package(Hal 0.1.0 REQUIRED CONFIG)

add_subdirectory(src)

# Define the application main and link to HAL.
add_executable(neutros_fc ${CMAKE_SOURCE_DIR}/src/fc_main/main.c)
target_link_libraries(neutros_fc PRIVATE Hal::stm32f4_hal scheduler)

# Strip the elf and prepare a bin to flash
add_custom_target(neutros_fc_binary ALL
    arm-none-eabi-objcopy -O binary neutros_fc neutros_fc.bin
    DEPENDS neutros_fc
)
