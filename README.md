# Neutros Flight Controller

## Project Structure

```
lib/
  hal/                  External HAL library (git submodule)

src/
  comms/
    c2/                 Inbound commands from mission computer
    telemetry/          Outbound state streaming

  flight/
    sensors/            Sensor drivers and abstraction (IMU, baro, mag, GPS)
    estimation/         Sensor fusion (attitude, position, velocity)
    control/            Cascaded PID loops (rate, attitude, position, altitude)
    mixing/             Airframe geometry → per-actuator commands
    actuators/          Motor and servo output (DShot, PWM)

  operations/
    mode/               Flight mode state machines (acro, stabilize, guided, RTL, ...)
    failsafe/           System health monitors and mode overrides

  param/
    static/             Board constants and boot defaults
    runtime/            Live-tunable parameter store

  system/
    blackboard/         Publish-subscribe data bus between tasks
    logging/            Deferred log sink dispatch
    schedule/           Cyclic executive and task registration
    time/               Timestamps and timing utilities
    watchdog/           Task health monitoring
```
