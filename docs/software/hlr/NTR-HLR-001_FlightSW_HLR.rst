Neutros Software High Level Requirements (HLR)
==============================================

.. hlr:: The flight software shall acquire IMU samples at a configured rate with bounded jitter.
   :id: HLR_001
   :status: draft
   :satisfies: SYS_001

.. hlr:: The flight software shall classify the IMU as faulted on loss of samples, out-of-range data, or self-test failure.
   :id: HLR_002
   :status: draft
   :satisfies: SYS_002

.. hlr:: The flight software shall expose IMU health (OK / Calibrating / Faulted) on the readiness telemetry.
   :id: HLR_003
   :status: draft
   :satisfies: SYS_003
