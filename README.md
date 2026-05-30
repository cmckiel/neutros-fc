# Neutros Flight Controller

⚠️ Caution: Work in Progress ⚠️

Neutros is a from scratch flight and autonomy software platform for drones and UAVs. This repo contains the flight controller. It is built from the registers up using [HAL](https://github.com/cmckiel/hal), a hardware abstraction layer for the STM32F4 flight computer Neutros is currently using.

The flight controller is still early, although it has achieved first flight.\* See here for a write-up: [First Uncontrolled Flight](https://cmckiel.github.io/posts/first-uncontrolled-flight/).

(\*No control laws were implemented and it crashed immediately. But it *did* leave the ground.)

## Project Status

There is an initial draft v0.1 Concept of Operations describing the project vision, scope, and initial definition of success. There is a rough sketch of an architecture: cyclic task execution w/ fixed frames, blackboard data plane for inter-task communication following pub/sub semantics, and a control pipeline capable of navigation, attitude and altitude stabilization, and waypoint following.

Beyond project aspirations, the code that exists now is the bare-minimum implementation necessary to leave the ground, if only for a moment. A C2 task maps single characters to vehicle commands, a motor control task takes commands and uses PWM to signal ESCs, a very basic round robin scheduler executes each task.

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
