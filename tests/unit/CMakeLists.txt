add_executable(
    desktop_unit_tests
    imu_mpu_6050_test.cpp
    time_util_test.cpp
    main.cpp
)

target_include_directories(
    desktop_unit_tests
    PRIVATE
)

target_link_libraries(
    desktop_unit_tests
    PRIVATE
    imu
    time_util
    GTest::gtest
    GTest::gtest_main
)

enable_testing()

include(GoogleTest)
gtest_discover_tests(desktop_unit_tests)
