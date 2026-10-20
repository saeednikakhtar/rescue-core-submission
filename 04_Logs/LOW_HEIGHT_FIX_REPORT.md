# Low-height detection correction

Developer: Saeed Nik. Native SITL only. Firmware SHA-256: `4bfa07c8b3f1febac6045f91058c5eab49b57801fd45eec16702940c54c2fe66`.

## Recorded fault

The original QGC flight reached 30.06 m Home with 7.67 m/s sink and -0.034 m/s² down acceleration. No acceleration-qualified sample occurred below 100 m during that descent. A separate later descent triggered at 77.67 m with 17.86 m/s²; it was a new qualification, not a delayed command. The original BIN snapshot is retained.

## Correction

The brief’s acceleration rule remains. The additional impact path requires confirmed descent (1 s / 2 m loss), height below 100 m Home, time to Home below 6 s, and sink above 5 m/s (or above 2 m/s below 30 m). A 0.2 s continuous dwell applies. Verified braking suppresses this extra path. A once-per-descent, bounded 0.5 s response grace lets a physically responding pilot pull-out establish braking clearance. It cannot veto the original acceleration rule or extend repeatedly.

No new parameters were added. Existing RSC_TTC_S is now 6 s and RSC_MIN_ALT also bounds the low-speed emergency envelope. Risk bits 32, 64 and 128 distinguish impact entry, braking clearance and response grace. TECS, existing attitude control and AP_Parachute still fly/release the aircraft.

This is an explicit extension to the PDF’s acceleration-only entry condition, made to address the requested steady-descent protection. Home is not terrain height. The margin and stopping estimate are native-model heuristics, not certified aircraft performance.

## Final native regression flights

| Case | Result | Capture Home / cause |
|---|---|---|
| takeoff40 | Passed | No capture |
| takeoff80 | Passed | No capture |
| level80 | Passed | No capture |
| brief_push | Passed | No capture |
| controlled_descent | Passed | No capture |
| fbwa_steady | Passed | 45.41 m / risk 50 |
| manual_steady | Passed | 70.27 m / risk 50 |
| pilot_pullout | Passed | No capture |
| climb_clear | Passed | No capture |
| disabled | Passed | No capture |
| engine_failure | Passed | 45.68 m / risk 50 |
| banked_descent | Passed | 18.96 m / risk 48 |
| pullout_near_margin | Passed | No capture |
| aborted_pullout | Passed | 34.92 m / risk 50 |

The seven required acceptance scenarios passed in eight separate native flights (MANUAL and AUTO are separate). Native BIN checks cover the original acceleration gate, existing TECS, stable envelope, five-second deadline, pilot handover, release output, motor cut and disarm.

The first final-run controlled-descent attempt hit only the test’s approach timeout before reaching 55 m, without any rescue intervention. The repeated case passed with a 60 s approach allowance. Both recordings remain available.

These 22 flights use direct MAVLink stimuli. They are not described as 22 mouse-flown QGC flights. The separate desktop campaign records real virtual-joystick input and any native mode/altitude preparation commands.

## Desktop QGC trial

Actual QGC input was verified with the temporary left-handed layout: pitch held its value after release. TAKEOFF, GUIDED altitude and source-mode preparation used direct MAVLink commands, and are not claimed as mouse commands. The neutral and gentle-descent intervals had no capture. Controlled descent crossed 100 m and reached the intended 70 m checkpoint without capture. The read-only checkpoint did not stop the held command, and the subsequent unattended approach reached the native ground. The overall desktop trial is not a passed rescue flight, and the requested complete ten-case desktop campaign remains pending. See low-height-review/qgc-desktop-trial.json and the original flight recording. The native 22-case verification is separate.

The checkpoint runner was corrected with an explicit --hold option. A separate native descent switched to LOITER at 79.39 m and subsequently held 78.50 m without RESCUE. This is a test-harness guardrail, not a firmware detector change. Original joystick settings were restored. The final prepared session is `live-20261016-171823-e9eed6bd`, disarmed on the runway, motor PWM 1000, QGC connected.
