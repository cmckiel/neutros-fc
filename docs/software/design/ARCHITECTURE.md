# Flight Controller Architecture

## Overview

The flight controller firmware runs on an STM32F4 under a cyclic scheduler. Every module is a scheduled task with the same shape: read from the blackboard, compute, write to the blackboard. The scheduler owns execution order, and the schedule table is the single source of truth for what runs when. A Raspberry Pi serves as the mission computer, handling waypoint planning, high-level decisions, and log storage. The two communicate over UART.

## Directory Structure

### `lib/`

External dependencies pulled in as git submodules. The primary occupant is the HAL library, which abstracts hardware peripherals (SPI, I2C, GPIO, timers) behind a portable interface. The HAL uses a separate `include/` and `src/` split because it is a distributable library — headers are the public API, source files are built per target. The FC repo consumes it via CMake `find_package` with a submodule fallback.

### `src/comms/`

Everything that crosses a wire to an external system.

- **`c2/`** — Command and control. Receives inbound commands from the mission computer (mode changes, parameter updates, guided setpoints). Reliability and validation matter here — a malformed command must never reach the flight pipeline.
- **`telemetry/`** — Outbound state streaming to the mission computer. Publishes attitude, position, velocity, mode, and system health at configurable rates. Also serves as a transport layer for the logging system when UART is configured as a log sink.

### `src/flight/`

The real-time control pipeline. These modules execute in sequence each cycle, forming the core sensor-to-actuator chain.

- **`sensors/`** — Drivers and abstraction for IMU, barometer, magnetometer, and GPS. Each sensor type defines a backend interface (function pointer vtable) so that chip-specific drivers can be swapped without touching consumers. Publishes raw calibrated data to the blackboard.
- **`estimation/`** — Sensor fusion. Reads sensor data from the blackboard and produces attitude, position, and velocity estimates. Complementary filter or EKF depending on requirements.
- **`control/`** — Cascaded PID control loops. Position → attitude → rate, plus altitude. Each layer reads setpoints and state from the blackboard and writes setpoints for the next layer down. A shared PID module provides the core controller with anti-windup and D-term filtering.
- **`mixing/`** — Maps roll/pitch/yaw/throttle commands from the control output into individual actuator commands based on airframe geometry (quad X, hex, tricopter, fixed-wing, etc.).
- **`actuators/`** — Hardware output. Motor drivers (DShot, PWM) and servo drivers. Owns protocol timing and signal generation. The mixer decides what each actuator should do; this module handles how.

### `src/operations/`

High-level decision making that sits above the flight pipeline.

- **`mode/`** — Flight mode state machine. Each mode (acro, stabilize, altitude hold, position hold, guided, RTL, land) generates the setpoints fed into the control pipeline. The guided mode accepts external setpoints from the mission computer. Only one mode is active at a time.
- **`failsafe/`** — Independent monitors for system health: RC link loss, data link loss, low battery, estimator divergence, geofence breach. Failsafe sits above mode in authority — it can force a mode transition (e.g., force RTL on link loss) regardless of operator input. Each monitor evaluates independently; any one can trigger.

### `src/param/`

All configuration values, both static and dynamic.

- **`static/`** — Board-level constants loaded at boot: pin mappings, peripheral assignments, hardware revision flags, default PID gains, failsafe thresholds.
- **`runtime/`** — Key-value parameter store that supports live adjustment over the C2 link. Validates incoming changes, applies them, and posts update events to the blackboard so consuming modules (e.g., control loops) pick up new gains without a restart.

### `src/system/`

Infrastructure that the rest of the codebase depends on. These modules are vehicle-agnostic.

- **`blackboard/`** — Publish-subscribe data bus. Every module reads and writes through the blackboard rather than calling into other modules directly. Entries should carry a timestamp or cycle count so consumers can detect stale data.
- **`logging/`** — Deferred write logging. Any module can call into the logger at any time, which enqueues the entry onto a ring buffer. When the logger's scheduled slot arrives, it drains as many entries as possible within its time budget. Log sinks are pluggable — UART now, onboard flash later, or both.
- **`schedule/`** — The cyclic executive. Owns the main loop. Tasks are registered as structs with metadata (call rate, function pointer). The scheduler runs all task init functions in dependency order at startup, then enters the cyclic loop. Should measure per-task execution time and flag overruns to the watchdog.
- **`time/`** — Timestamps, cycle counting, and timing utilities. Provides the common time base for the blackboard, logging, and scheduler.
- **`watchdog/`** — Monitors task health by tracking heartbeats and execution timing. When a task stops running or consistently overruns, the watchdog flags the condition. The watchdog mechanism lives here; the *response* to a watchdog event (e.g., forcing RTL) lives in `operations/failsafe/`.

## Execution Model

1. `main()` initializes the scheduler.
2. The scheduler runs all task `init()` functions in dependency order (sensors before estimator, estimator before control, etc.).
3. The scheduler enters the cyclic loop. Each tick, tasks execute in schedule-table order at their configured rates.
4. Every task follows the same pattern: read blackboard → compute → write blackboard.
5. The schedule table implicitly defines data flow ordering — producers run before consumers within a cycle.

## Intermodule Communication

- **Blackboard** is the primary communication mechanism. Modules that run at different rates or have no direct dependency communicate exclusively through the blackboard.
- **Logging** is the exception: any module can enqueue a log entry at any time via a direct call into the logger's ring buffer. This is safe because the cyclic scheduler is non-preemptive — no locking required. This assumption should be revisited if interrupts or DMA callbacks ever produce log entries.

## FC / Mission Computer Split

| Responsibility | Owner |
|---|---|
| Stabilization and control loops | FC |
| State estimation and sensor fusion | FC |
| Failsafe and safety-critical behavior | FC |
| Actuator output | FC |
| RTL and land (must work without Pi) | FC |
| Waypoint following and mission planning | Pi |
| Path planning and obstacle avoidance | Pi |
| Log storage and forwarding | Pi |
| High-level autonomy and computer vision | Pi |

The Pi sends position/velocity setpoints over UART. The FC's guided mode tracks them. From the FC's perspective, the source of a setpoint is irrelevant — it could be the Pi, an RC transmitter, or an internal failsafe. If the Pi goes silent, the FC detects link loss and failsafe handles it independently.
