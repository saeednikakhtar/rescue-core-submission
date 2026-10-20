# RESCUE implementation and design decisions

Developer: Saeed Nik. ArduPlane 4.6.3 native SITL, firmware 3.0.1.

## 1. Context and objective

RESCUE is compiled into the flight firmware as mode 26 and a 50 Hz supervisor. The existing TECS energy controller, attitude controllers and AP_Parachute perform recovery and release. No application panel or Python service is required for this behavior.

## 2. General behavior

During confirmed unsafe descent, send warning messages at about 1 Hz below 200 m Home without changing mode. If qualifying descent continues below 100 m, capture RESCUE while armed and flying with a usable parachute. Recover to 200 m using existing TECS, confirm continuous stable flight, then allow five seconds for pilot acknowledgment. A valid mode change hands back control; no response requests the parachute, motor cut and disarm. The separately disclosed steady-descent and optional stick-acknowledgment additions are detailed below.

## 3. Parameters

Only the 26 parameters added by this implementation are documented here. The .parm file also contains existing simulator/model settings needed to reproduce the flights. Firmware RSC_ENABLE defaults to 0; the test profile sets it to 1.

| Parameter | Test value | Unit | Function |
|---|---:|---|---|
| RSC_ENABLE | 1 | 0/1 | Enable the supervisor. Firmware default is 0; the test profile sets 1. |
| RSC_ALT_WARN | 200 | m Home | Warning height and TECS recovery target. One shared value avoids contradictory warning and target settings. |
| RSC_ALT_TRIG | 100 | m Home | Automatic capture threshold while qualifying descent continues. Armed and in-flight checks also apply. |
| RSC_ACC_Z | 5 | m/s^2 | Gravity-compensated downward acceleration for the brief’s entry path. The additional low-height impact path can act with lower acceleration; see section 4.2. |
| RSC_HAND_S | 5 | s | Pilot response window beginning at confirmed stable recovery. |
| RSC_DWELL_S | 0.2 | s | Persistence of the qualified acceleration condition. A separate fixed gate requires continuous measured descent for at least 1 s and 2 m height loss. |
| RSC_ALT_HYST | 10 | m | Height hysteresis used when clearing a warning. Limits chatter around the warning boundary. |
| RSC_SINK | 5 | m/s | Minimum sink for the additional low-height impact path. Sink alone is insufficient: confirmed descent, short crossing time and entry dwell are required. |
| RSC_UN_ANG | 60 | deg | Severe-attitude diagnostic and recovery failure threshold. Does not bypass the required acceleration at entry. |
| RSC_UN_RATE | 90 | deg/s | Rotation diagnostic and recovery failure threshold. Does not independently trigger capture. |
| RSC_TTC_S | 6 | s | Additional entry margin below ALT_TRIG during confirmed sink above RSC_SINK, unless actual braking predicts stopping above 5 m Home. This is an explicit extension to the acceleration rule. |
| RSC_STAB_S | 2 | s | Continuous time for which every stable-flight criterion must hold. |
| RSC_ALT_TOL | 5 | m | Allowed height error about the recovery target. Defines the stable height band. |
| RSC_VZ_TOL | 1 | m/s | Maximum absolute vertical speed accepted as level flight. |
| RSC_ANG_MAX | 10 | deg | Maximum absolute roll and pitch accepted as stable. |
| RSC_ST_RATE | 15 | deg/s | Maximum body rotation accepted as stable. Rejects transient zero-angle crossings. |
| RSC_RECOV_S | 90 | s | Overall deadline to achieve stable recovery, measured from capture. |
| RSC_FAIL_S | 3 | s | Persistent excessive sink or severe instability allowed after the initial recovery grace period. |
| RSC_MIN_ALT | 30 | m Home | Parachute threshold during recovery. Below this height the additional TTC entry guard also accepts sink above 2 m/s. |
| RSC_PROG_S | 10 | s | Progress window below the target region. Evaluates actual height and specific-energy-height change. |
| RSC_PROG_M | 3 | m | Minimum height gain OR equivalent total-energy-height gain in a progress window. Airspeed recovery can count. |
| RSC_IMPACT_S | 2 | s | Urgent predicted Home-datum crossing threshold during descent above 2 m/s. |
| RSC_REARM_S | 10 | s | Minimum cooldown after handover. A clear-risk interval is also required; imminent danger can override cooldown. |
| RSC_STK_EN | 1 | 0/1 | Optional deliberate-stick/throttle acknowledgment. Set 0 for mode-change-only handover. |
| RSC_STK_US | 120 | us | Required raw mapped-channel change from the stabilization baseline. Rejects small noise and unchanged held input. |
| RSC_STK_S | 0.3 | s | Continuous deliberate-input duration with valid fresh input before handover to FBWA. |

The additional settings make persistence, stable flight, recovery failure and pilot input acknowledgment explicit. They are supervisor settings; they do not replace TECS or the existing attitude controllers. Attitude or rotation alone never captures control. RSC_SINK and RSC_TTC_S jointly define the added low-height safeguard; no new parameters were introduced for this correction.

## 4. Detailed phase requirements

### 4.1 MONITOR to WARNING

The 50 Hz supervisor uses gravity-compensated earth-frame NED acceleration, filtered at 5 Hz. Positive Az and Vz in the RSC log mean DOWN. Normal upward acceleration has the opposite sign. Confirmed descent requires Vz greater than 0.5 m/s continuously for at least 1 s and at least 2 m measured altitude loss. After this gate, Az must exceed RSC_ACC_Z continuously for RSC_DWELL_S. The default threshold remains 5 m/s^2. A transient shorter than one second cannot meet the descent gate.

### 4.2 WARNING to RESCUE

The original acceleration branch is retained. A separate safeguard now captures a dangerous steady descent below 100 m Home: descent must exceed 0.5 m/s continuously for at least 1 s and lose 2 m, current sink must exceed RSC_SINK (5 m/s), or exceed 2 m/s below RSC_MIN_ALT (30 m), and Home height / sink must be below RSC_TTC_S (6 s). The condition must persist for RSC_DWELL_S (0.2 s). It is logged as risk bit 32 and a dedicated predicted-impact message. This is a deliberate extension to the brief’s acceleration-only gate, added after a real QGC flight reached 30 m Home at 7.67 m/s with Az near zero.

The safeguard yields to verified braking only when both filtered gravity-compensated acceleration and a 1 Hz filtered derivative of EKF down speed indicate upward deceleration above 1 m/s², and h − Vz²/(2 × braking) exceeds 5 m Home. A once-per-descent grace of at most 0.5 s can allow an incipient pull-out to establish this margin: either measured upward pitch rate exceeds 5 deg/s with Az below -0.5 m/s² and filtered sink acceleration below -0.25 m/s², or fresh MANUAL/FBWA input requests at least 20% nose-up with measured pitch rate above 0.5 deg/s and Az below 0.5 m/s². Height must exceed MIN_ALT and TTC must exceed IMPACT_S. It cannot be extended by held sticks or repeated pulses and does not suppress the acceleration rule. Braking clearance and grace are logged as bits 64 and 128. The short-term constant-deceleration estimate is a heuristic, not an aircraft performance guarantee. It never prevents the original acceleration rule. Normal shallow descent, climb, short transients and rotation alone do not satisfy the additional gate. All trend gates reset when descent ends or navigation sampling is interrupted. Capture remains mode-independent, including TAKEOFF. Warning clears after 0.5 s without either qualifying path.

### 4.3 RESCUE recovery

Existing TECS manages the 200 m Home height target and airspeed. Existing roll/pitch/yaw controllers level the wings. During the main climb the full available throttle floor respects existing output/power/slew limits; it is released near the target so TECS can settle. No separate recovery pitch/throttle law is added. Full throttle alone is neither proof of climb nor proof of failure: height or equivalent energy-height gain counts as progress.

On emergency entry only, the existing TECS reset seeds its pitch-demand slew state inside the configured pitch bounds. This avoids spending several seconds slewing an internal demand from a near-vertical measured dive toward a boundary that Plane already clips. Normal mode engagement keeps the original TECS reset behavior; energy control, anti-stall logic and acceleration limiting remain active.

### 4.4 STABILIZED handover

For 2 continuous seconds: altitude 200 +/- 5 m, absolute vertical speed at most 1 m/s, absolute roll/pitch at most 10 degrees, body rate at most 15 deg/s and airspeed at least the existing minimum. Then send the stabilized signal and start RSC_HAND_S. An explicit mode change is accepted. Optional fresh stick/throttle acknowledgment selects FBWA; unchanged held input, noise and stale data do not count. RSC_STK_EN=0 retains mode-change-only acknowledgment.

### 4.5 Parachute

No pilot response within 5 s requests AP_Parachute, cuts the motor and disarms. In the tested profile the release output activates after 500 ms; modeled canopy inflation takes 1.5 s. After release start a fresh simulation to repack the modeled chute.

## 5. Edge cases and safety

The takeoff false capture was traced to a roughly 0.32 m temporary height loss at 84.96 m, with Az 6.66 m/s^2 and Vz 1.92 m/s, during normal level-off toward the 80 m target. The former 0.2 s criterion was too short. The corrected entry requires the measured descent trend and the specified acceleration threshold; the takeoff target was not increased to hide the bug.

Once RESCUE already owns the controls, existing bounded recovery watchdogs remain: continued sink, severe instability, insufficient height/energy progress, low altitude, urgent predicted Home crossing, navigation data loss and overall recovery timeout. These are failure checks after capture, not independent entry conditions. The additional impact gate can override cooldown, preventing a fresh low-height dive during the handover interval from being ignored. RSC_ENABLE=0 disables this supervisor.

The acceleration-only limitation is now addressed by the explicit low-height safeguard in section 4.2. Height uses Home, not measured terrain; rising terrain, delayed estimates, very late dive onset and airframe limits can still prevent recovery. The six-second margin is tested in the native model and is not flight-certified. An intentional fast descent into this envelope also triggers: stick intent cannot prove sufficient ground clearance. The simple native model can rebound and fly after impact, so subsequent stabilization cannot validate that trial as a successful rescue.

RSC records contain the decision values, state and event. MODE, TECS, RCOU and ARM provide the independent mode, controller, release and disarm evidence. Named telemetry RSC_DLOSS and RSC_DTIME expose the measured descent loss and duration.

## 6. Deliverables

The five required outputs are indexed in ../README.md. The complete native parameter profile is ../02_Parameters/rescue.parm. Only the 26 parameters added by this implementation are described here; existing ArduPlane/model values remain in the complete profile for reproduction. The unchanged eight-page English response is RESCUE_Response_EN_3.0.1.docx.

## 7. Required SITL scenarios

| Number | Scenario | Evidence |
| --- | --- | --- |
| 1 | Warning below 200 m, mode unchanged | warning.BIN |
| 2 | Continued descent below 100 m; automatic capture from MANUAL and AUTO | manual.BIN, auto.BIN |
| 3 | TECS recovery to 200 m, wings level and safe speed, stabilized signal | recovery.BIN |
| 4 | Pilot changes to FBWA within five seconds; no parachute | handover.BIN |
| 5 | No pilot response; parachute, motor cut and disarm | no_pilot.BIN |
| 6 | Aircraft cannot recover; safe degradation to parachute | failed.BIN |
| 7 | RSC_ENABLE=0; no supervisor intervention | disabled.BIN |

These eight original recordings are in ../04_Logs/bin/. Phase_Analysis.md and acceptance.json identify the native RSC, MODE, TECS, RCOU and ARM evidence. Test stimuli use standard MAVLink commands and actual native dynamics; sensor values or supervisor state are not invented. These are not eight mouse-flown QGC trials. Engine failure is applied after qualifying capture to isolate failed-recovery behavior.

Additional engineering regressions covered normal takeoff and mode changes, controlled descent, steady sink and short transients. The original verified firmware passed 22 unique native cases: 14 regressions and eight acceptance flights. A complete ten-case attended desktop-QGC campaign is not claimed. Physical flight-controller and parachute qualification remain separate from native SITL evidence.
