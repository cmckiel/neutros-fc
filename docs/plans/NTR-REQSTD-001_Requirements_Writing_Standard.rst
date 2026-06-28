
```
SYS-001, SYS-002, ...                    # system requirements, flat
HLR-001, HLR-002, ...                    # software high-level, flat
LLR-IMU-001, LLR-EST-001, LLR-MIX-001    # low-level, scoped by subsystem
DD-IMU-001                                # design docs, scoped by subsystem
TC-IMU-001, TC-EST-001                    # test cases, scoped by subsystem
HAZ-001                                   # hazards, flat (when you add them)
```

Requirement Identifiers. Every requirement, design element, hazard, and test case in this project has a unique, stable identifier.
System requirements use the form SYS-NNN where NNN is a zero-padded three-digit number assigned sequentially.
Software high-level requirements use HLR-NNN. Software low-level requirements use LLR-XXX-NNN where XXX is a three-letter subsystem tag (e.g., IMU, EST, MIX, MOD, COM).
Design document sections use DD-XXX-NNN, test cases use TC-XXX-NNN, and hazards use HAZ-NNN. Identifiers are permanent: once assigned, an identifier is never reused or renumbered, even if the requirement it names is deleted.
Gaps in the numeric sequence are expected and acceptable. New identifiers are assigned by incrementing past the highest existing number within the relevant scope.
Subsystem tags are drawn from a controlled list maintained in this document; adding a new subsystem tag is a deliberate act and requires updating the list below.

Tag Subsystem
IMU Inertial measurement unit driver and sample acquisition
EST Attitude and position estimation
MIX Control mixer and motor command generation
MOD Mode logic and mode transitions
COM Command link, telemetry, and GCS protocol
SYS System services (scheduler, watchdog, time, logging)
HAL Hardware abstraction layer

The full doc, when finished, has maybe four or five short sections:

Requirement Identifiers — the paragraph and table I just gave you.
Requirement Wording — one paragraph. Every requirement uses "shall", is atomic (one testable claim per requirement), is verifiable, and avoids vague terms like "appropriate", "as needed", "fast", "user-friendly".
Required Attributes — what every requirement carries besides its text: ID, rationale, verification method (test / analysis / inspection / demonstration), parent link, status.
Lifecycle and Status Values — the allowed status values (e.g., draft, approved, implemented, verified, deprecated) and what each means.
Change Discipline — IDs are permanent, deleted requirements get status deprecated rather than being removed, substantive changes bump a revision field.

Keep the whole thing under two pages. It's a contract with future-you, not a textbook.
