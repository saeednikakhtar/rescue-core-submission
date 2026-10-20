# Core submission — public download test

Developer: Saeed Nik. Checked on 20 October 2026.

The core ZIP was downloaded from the independent public rescue-core-submission release, extracted to a fresh folder and checked against every sealed payload checksum. ZIP CRC, the patch, 30 modified files, original eight acceptance BINs, complete parameters and the unchanged English Word response were verified. No Python interpreter/dependency distribution, Flight Lab panel, application launcher, QGC installer or helper script was present. The only executable is the requested compiled native SITL flight firmware.

The complete corresponding-source archive was separately downloaded and its SHA-256 checked. All 30 modified firmware sources matched the core submission. GitHub's main-branch source ZIP was also downloaded and every sealed payload checksum matched. The downloaded patch applied to the pinned ArduPlane base and reproduced all 30 modified sources, allowing ordinary CRLF/LF text representation.

The downloaded arduplane.exe loaded with a PATH containing only Windows system directories, using the native DLLs beside it. An external MAVLink test client, kept outside the delivered package, drove the native simulator on separate local ports and parsed the new DataFlash BIN recordings. It did not use or reset the running Flight Lab web service or QGC session.

| Required scenario | Fresh native flight | Result |
| --- | --- | --- |
| 1 Warning, no mode change | warning | PASS |
| 2 Capture from MANUAL | manual | PASS |
| 2 Capture from AUTO | auto | PASS |
| 3 Existing TECS recovery and stable flight | recovery | PASS |
| 4 Pilot handover to FBWA | handover | PASS |
| 5 No pilot response, parachute | no_pilot | PASS |
| 6 Recovery failure, parachute | failed | PASS |
| 7 RSC_ENABLE=0 | disabled | PASS |
| Additional disarmed-ground entry guard | ground | PASS |

This is seven required scenarios in eight new native flights, plus one ground guard. The tests use repeatable standard MAVLink stimuli and native physics; they are not claimed as mouse-flown QGC trials. Python is an external verification-client dependency, not part of the delivered firmware or its native runtime.

The existing rescue-flight-lab repository was read-only during this work. Its main commit, release v3.0.2 metadata and every existing release asset ID, size, SHA-256 and update timestamp matched the before/after snapshot. No existing release was edited and no existing asset was replaced.

Core ZIP SHA-256: `1fff06c2bbe2fd8a28f648bd7b6be06a9581dbbd83b20f895935ff16feda3cb7`

Firmware SHA-256: `4bfa07c8b3f1febac6045f91058c5eab49b57801fd45eec16702940c54c2fe66`

Core-Download-Verification.json supplies the detailed assertions and native BIN hashes. The separate core test-log archive contains the nine new native recordings and case results. This is a verification on the supplied Windows system; no new physical flight-controller or real-aircraft qualification is claimed.
