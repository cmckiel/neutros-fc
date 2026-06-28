IMU Low Level Requirements (LLR)
================================

.. llr:: The driver shall initialize the device with configured gyro range, accel range, and DLPF bandwidth, and verify the WHO_AM_I register on startup.
   :id: LLR_IMU_001
   :status: draft
   :satisfies: HLR_001

.. llr:: The driver shall produce timestamped samples at the configured ODR with jitter ≤ X µs.
   :id: LLR_IMU_002
   :status: draft
   :satisfies: HLR_001

.. llr:: The driver shall report a bus error fault on N consecutive failed transactions.
   :id: LLR_IMU_003
   :status: draft
   :satisfies: HLR_002

.. llr:: The driver shall report a data fault when raw samples remain bit-identical for longer than T ms (stuck-sensor detection).
   :id: LLR_IMU_004
   :status: draft
   :satisfies: HLR_002
