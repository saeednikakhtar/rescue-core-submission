# Native BIN phase analysis

Developer: Saeed Nik. ArduPlane 4.6.3 native Windows SITL, release 3.0.1.

Firmware SHA-256: `4bfa07c8b3f1febac6045f91058c5eab49b57801fd45eec16702940c54c2fe66`.

The seven requested scenarios were checked in eight independent flights. MANUAL and AUTO capture are separate runs. These flights use standard MAVLink stimuli and native aircraft dynamics; they are not claimed as eight complete mouse-flown QGroundControl trials.

Time is native simulation time from TimeUS, in seconds. Alt is Home-relative height at the RSC decision. Az is gravity-compensated acceleration positive DOWN, in m/s². Vz is sink positive DOWN, in m/s. A parachute row is the supervisor request time; the actuator output follows the configured 0.5 s delay.

## Where each phase is visible

- RSC: supervisor state/event, decision height, acceleration, sink, attitude, airspeed and risk flags.
- MODE: automatic mode capture and pilot handover, including the mode-change reason.
- TECS: the existing controller is active during recovery; this is not a replacement pitch/throttle law.
- RCOU: motor PWM and the parachute release channel.
- ARM: armed/disarmed state around release.

Risk flags may be combined: 1 acceleration, 2 sink, 4 attitude, 8 rotation, 16 time to Home, 32 additional impact-entry guard, 64 verified braking clearance, 128 bounded pull-out response grace. A risk bit alone is not necessarily a capture event.

## warning

Original file: [bin/warning.BIN](bin/warning.BIN). Result: **PASS**.
BIN SHA-256: `fe955eff64fa957a8091720587f0195b533ebef13761d7b43d880830becf7163`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 80.464 | WARNING | 196.55 | 8.103 | 17.005 | 3 |
| 81.125 | Clear | 188.72 | -61.374 | -3.229 | 0 |

Checked from BIN: original_bin_present, warning, no_capture, warning_acceleration.

## manual

Original file: [bin/manual.BIN](bin/manual.BIN). Result: **PASS**.
BIN SHA-256: `10a97f30ac78aef321efbf61a84cf7af3336da00840c150878180c7aa5e171b2`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 69.465 | WARNING | 136.51 | 5.726 | 5.248 | 3 |
| 72.045 | RESCUE capture | 99.52 | 7.437 | 23.963 | 51 |
| 102.305 | STABILIZED | 197.20 | -2.516 | -0.479 | 0 |
| 102.325 | Pilot handover | 197.21 | -2.652 | -0.536 | 0 |
| 112.325 | Clear | 209.40 | 0.069 | -1.207 | 0 |

Checked from BIN: original_bin_present, capture, capture_acceleration_above_threshold, capture_sustained_measured_descent, capture_below_100, specified_source_mode, stabilized, stable_envelope, existing_tecs_used, airspeed_above_minimum, model_stall_threshold_not_crossed, handover_no_chute.

## auto

Original file: [bin/auto.BIN](bin/auto.BIN). Result: **PASS**.
BIN SHA-256: `0d658d282b30dadf0b382d3f41c9d4452eaad8c6febe0e657f771e9c4b24a366`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 70.545 | WARNING | 128.54 | 5.825 | 6.431 | 3 |
| 72.464 | RESCUE capture | 99.71 | 10.779 | 24.664 | 51 |
| 111.065 | STABILIZED | 196.99 | -1.288 | -0.485 | 0 |
| 111.105 | Pilot handover | 197.01 | -1.429 | -0.546 | 0 |
| 121.105 | Clear | 208.84 | 0.005 | -1.176 | 0 |

Checked from BIN: original_bin_present, capture, capture_acceleration_above_threshold, capture_sustained_measured_descent, capture_below_100, specified_source_mode, stabilized, stable_envelope, existing_tecs_used, airspeed_above_minimum, model_stall_threshold_not_crossed, handover_no_chute.

## recovery

Original file: [bin/recovery.BIN](bin/recovery.BIN). Result: **PASS**.
BIN SHA-256: `39d10de1c26c92e2f129233fb439aed2c4df26516b6835c115e8a7361f060a60`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 68.384 | WARNING | 141.43 | 16.010 | 12.232 | 3 |
| 68.985 | Clear | 136.35 | -8.459 | 3.698 | 64 |
| 69.445 | WARNING | 134.47 | 6.793 | 5.508 | 3 |
| 71.925 | RESCUE capture | 99.87 | 7.490 | 23.078 | 51 |
| 101.905 | STABILIZED | 197.36 | -1.059 | -0.406 | 0 |
| 101.925 | Pilot handover | 197.37 | -1.214 | -0.436 | 0 |
| 111.944 | Clear | 209.48 | -0.247 | -1.229 | 0 |

Checked from BIN: original_bin_present, capture, capture_acceleration_above_threshold, capture_sustained_measured_descent, capture_below_100, stabilized, stable_envelope, existing_tecs_used, airspeed_above_minimum, model_stall_threshold_not_crossed, handover_no_chute.

## no_pilot

Original file: [bin/no_pilot.BIN](bin/no_pilot.BIN). Result: **PASS**.
BIN SHA-256: `e26323af64ddf0508deacce8c2d710e0aed437e849f41a5508b1c6f0b2b6a6fe`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 68.384 | WARNING | 141.47 | 15.821 | 12.216 | 3 |
| 68.985 | Clear | 136.45 | -8.565 | 3.780 | 64 |
| 69.465 | WARNING | 134.43 | 6.387 | 5.641 | 3 |
| 71.925 | RESCUE capture | 99.96 | 7.416 | 23.083 | 51 |
| 101.905 | STABILIZED | 197.29 | -1.073 | -0.408 | 0 |
| 106.925 | No-response parachute | 199.90 | -0.241 | -0.237 | 0 |

Checked from BIN: original_bin_present, capture, capture_acceleration_above_threshold, capture_sustained_measured_descent, capture_below_100, stabilized, stable_envelope, existing_tecs_used, airspeed_above_minimum, model_stall_threshold_not_crossed, no_false_pilot_ack, no_pilot_chute, five_second_deadline, release_output, motor_cut, disarmed.

## handover

Original file: [bin/handover.BIN](bin/handover.BIN). Result: **PASS**.
BIN SHA-256: `25189ba88065915851c0cdb3b35acf970db8472261dcaa56ea719a20610d447f`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 68.384 | WARNING | 141.43 | 15.924 | 12.231 | 3 |
| 68.985 | Clear | 136.42 | -7.821 | 3.815 | 64 |
| 69.445 | WARNING | 134.45 | 6.326 | 5.651 | 3 |
| 71.925 | RESCUE capture | 99.83 | 7.485 | 23.072 | 51 |
| 102.025 | STABILIZED | 197.16 | -1.467 | -0.385 | 0 |
| 102.045 | Pilot handover | 197.16 | -1.675 | -0.426 | 0 |
| 112.044 | Clear | 209.32 | -0.025 | -1.238 | 0 |

Checked from BIN: original_bin_present, capture, capture_acceleration_above_threshold, capture_sustained_measured_descent, capture_below_100, stabilized, stable_envelope, existing_tecs_used, airspeed_above_minimum, model_stall_threshold_not_crossed, handover_no_chute.

## failed

Original file: [bin/failed.BIN](bin/failed.BIN). Result: **PASS**.
BIN SHA-256: `45ffd512c897974a88645d55f8a4ec86f6f2da5030b05055a0fb7a4c0664afe2`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| 68.384 | WARNING | 141.34 | 26.770 | 13.664 | 3 |
| 69.025 | Clear | 134.69 | -9.617 | 4.752 | 64 |
| 69.545 | WARNING | 132.05 | 5.866 | 6.488 | 3 |
| 71.845 | RESCUE capture | 99.62 | 7.556 | 22.514 | 51 |
| 81.945 | No-progress parachute | 73.54 | -0.986 | 4.133 | 0 |

Checked from BIN: original_bin_present, capture, capture_acceleration_above_threshold, capture_sustained_measured_descent, capture_below_100, failure_chute, release_output, motor_cut, disarmed.

## disabled

Original file: [bin/disabled.BIN](bin/disabled.BIN). Result: **PASS**.
BIN SHA-256: `c1cf1e872053879f0ddb3c8a588ebd565adf53e19c7f390f0ff2940e5277d3c8`.

| Native time s | Event | Home m | Az down m/s² | Sink m/s | Risk |
|---:|---|---:|---:|---:|---:|
| — | No RESCUE transitions | — | — | — | — |

Checked from BIN: original_bin_present, no_intervention.

## Reference five-second decision

In the original no_pilot acceptance flight, capture occurred at 71.924552 s and 99.96 m Home with Az 7.416 m/s². Stable recovery was declared at 101.905055 s and 197.29 m. The no-response request was logged at 106.924713 s, a 5.019658 s interval. The approximately 20 ms difference is one supervisor scheduling step; RSC_HAND_S remains 5 s. Release output, motor cut and disarm are independently checked.

## Disclosed extensions

The acceleration path in the brief remains. The extra steady-descent guard can also capture below 100 m when continuous descent has lasted 1 s with 2 m loss, sink exceeds 5 m/s (or 2 m/s below 30 m), and time to Home is below 6 s for 0.2 s. Verified braking and a once-per-descent response grace can defer only this extra path. Optional fresh deliberate stick/throttle acknowledgment selects FBWA; RSC_STK_EN=0 retains mode-change-only handover.

## Additional recorded video flight

The separate [media-flight/result.json](media-flight/result.json) is a repeat of scenario 5 on the identical binary, extended to full modeled canopy inflation and model ground contact. Its [original BIN](media-flight/no-pilot-complete.BIN) is retained. It supports the recorded demonstration and does not increase the count of unique acceptance scenarios.

## Verification limits

Fourteen additional native regressions are recorded in low-height-verification.json and the complete release. A retained controlled-descent approach-timeout attempt was followed by a successful repeat with a longer approach allowance on the same firmware. The full ten-case attended QGC desktop campaign remains incomplete. Home is not measured terrain clearance. No physical-controller or real-parachute qualification is claimed.
