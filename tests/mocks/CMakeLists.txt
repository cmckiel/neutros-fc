add_library(hal_mock STATIC
    mock_hal_globals.c
    mock_hal_gpio.c
    mock_hal_uart.c
    mock_hal_i2c.c
    mock_hal_pwm.c
    mock_hal_systick.c
    mock_hal_system.c
    mock_hal_metadata.c
)

target_include_directories(hal_mock
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hal_mock
    PUBLIC
    hal_interface
)
