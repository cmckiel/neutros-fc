Requirements trace smoke test
=============================

.. sysreq:: Attitude control loop shall run at 100 Hz
   :id: SYS_001
   :status: open

   The flight controller shall execute the attitude control loop at a
   minimum rate of 100 Hz.

.. hlr:: Scheduler provides 100 Hz tick
   :id: HLR_001
   :status: open
   :satisfies: SYS_001

   The scheduler shall provide a 100 Hz periodic tick to drive the
   attitude control loop.

Trace table
-----------

.. needtable::
   :columns: id, title, satisfies, status
   :types: sysreq, hlr
   :style: table
