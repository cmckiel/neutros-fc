IMU Driver State Machine
========================

.. design:: IMU Driver State Machine
   :id: DD_IMU_001
   :realizes: LLR_IMU_001, LLR_IMU_003, LLR_IMU_004

   The driver operates as a four-state machine: UNINIT, INIT, READY, FAULTED.
   Transitions are defined in Figure 2. The FAULTED state is entered on any
   of: WHO_AM_I mismatch, N consecutive bus errors, or stuck-sample detection.
