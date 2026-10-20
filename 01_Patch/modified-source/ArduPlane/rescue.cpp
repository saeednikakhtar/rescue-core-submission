// RESCUE integration: Saeed Nik.
// The supervisor decides when to intervene; the existing flight controllers
// remain responsible for the aircraft response.
#include "Plane.h"
#include "rescue.h"

const AP_Param::GroupInfo Rescue::var_info[] = {
    // @Param: ENABLE
    // @DisplayName: Enable fixed-wing emergency recovery
    // @Description: Enables warning, recovery and parachute handover logic. Requires a configured parachute. SITL experimental feature.
    // @Values: 0:Disabled,1:Enabled
    // @User: Advanced
    AP_GROUPINFO_FLAGS("ENABLE", 1, Rescue, enable, 0, AP_PARAM_FLAG_ENABLE),
    // @Param: ALT_WARN
    // @DisplayName: Warning and recovery target altitude
    // @Description: Height above Home below which sustained unsafe descent generates a warning. Also the TECS recovery target.
    // @Units: m
    // @Range: 10 10000
    // @User: Advanced
    AP_GROUPINFO("ALT_WARN", 2, Rescue, alt_warn, 200),
    // @Param: ALT_TRIG
    // @DisplayName: Automatic recovery altitude
    // @Description: Height above home below which sustained falling captures any fixed-wing mode. Must be below ALT_WARN.
    // @Units: m
    // @Range: 5 9999
    // @User: Advanced
    AP_GROUPINFO("ALT_TRIG", 3, Rescue, alt_trig, 100),
    // @Param: ACC_Z
    // @DisplayName: Downward acceleration threshold
    // @Description: Gravity-compensated NED down acceleration threshold. Filtered at 5Hz. Requires confirmed descent above 0.5m/s lasting 1s with at least 2m measured height loss; a brief takeoff level-off is not a fall.
    // @Units: m/s/s
    // @Range: 0.1 50
    // @User: Advanced
    AP_GROUPINFO("ACC_Z", 4, Rescue, acc_z, 5),
    // @Param: HAND_S
    // @DisplayName: Pilot handover window
    // @Description: Seconds after stable recovery for a flight-mode change or deliberate fresh stick movement, otherwise deploy parachute.
    // @Units: s
    // @Range: 0.5 60
    // @User: Advanced
    AP_GROUPINFO("HAND_S", 5, Rescue, hand_s, 5),
    // @Param: DWELL_S
    // @DisplayName: Falling persistence time
    // @Description: Seconds for which a qualifying unsafe-descent condition must remain continuously true before declaring a fall.
    // @Units: s
    // @Range: 0.02 2
    // @User: Advanced
    AP_GROUPINFO("DWELL_S", 6, Rescue, dwell_s, 0.2),
    // @Param: ALT_HYST
    // @DisplayName: Warning altitude hysteresis
    // @Description: Extra altitude above ALT_WARN required to clear warning by altitude. A missing qualified acceleration condition also clears warning after 0.5s.
    // @Units: m
    // @Range: 1 100
    // @User: Advanced
    AP_GROUPINFO("ALT_HYST", 7, Rescue, alt_hyst, 10),
    // @Param: STAB_S
    // @DisplayName: Stable flight persistence time
    // @Description: Time all height, vertical-speed, attitude and airspeed stability conditions must remain true.
    // @Units: s
    // @Range: 0.2 10
    // @User: Advanced
    AP_GROUPINFO("STAB_S", 8, Rescue, stable_s, 2),
    // @Param: ALT_TOL
    // @DisplayName: Stable height tolerance
    // @Description: Maximum absolute height error from ALT_WARN considered stable.
    // @Units: m
    // @Range: 1 20
    // @User: Advanced
    AP_GROUPINFO("ALT_TOL", 9, Rescue, alt_tol, 5),
    // @Param: VZ_TOL
    // @DisplayName: Stable vertical speed tolerance
    // @Description: Maximum absolute NED vertical speed considered stable.
    // @Units: m/s
    // @Range: 0.1 3
    // @User: Advanced
    AP_GROUPINFO("VZ_TOL", 10, Rescue, vz_tol, 1),
    // @Param: ANG_MAX
    // @DisplayName: Stable attitude tolerance
    // @Description: Maximum absolute roll and pitch angle considered level flight.
    // @Units: deg
    // @Range: 1 20
    // @User: Advanced
    AP_GROUPINFO("ANG_MAX", 11, Rescue, angle_max, 10),
    // @Param: RECOV_S
    // @DisplayName: Maximum recovery time
    // @Description: If stable recovery has not been achieved by this time after capture, deploy parachute.
    // @Units: s
    // @Range: 5 180
    // @User: Advanced
    AP_GROUPINFO("RECOV_S", 12, Rescue, recovery_s, 90),
    // @Param: FAIL_S
    // @DisplayName: Continued descent failure time
    // @Description: Deploy if descent above 3m/s persists this long after the first 2 seconds of recovery.
    // @Units: s
    // @Range: 0.5 10
    // @User: Advanced
    AP_GROUPINFO("FAIL_S", 13, Rescue, fail_s, 3),
    // @Param: MIN_ALT
    // @DisplayName: Emergency parachute altitude
    // @Description: Deploy if still descending below this height above Home during recovery. Also permits the added predicted-impact entry path at sink above 2m/s below this height, with confirmed descent and TTC_S dwell. Not terrain height.
    // @Units: m
    // @Range: 1 1000
    // @User: Advanced
    AP_GROUPINFO("MIN_ALT", 14, Rescue, min_alt, 30),
    // @Param: REARM_S
    // @DisplayName: Recovery rearm cooldown
    // @Description: Minimum time after pilot handover or another mode exit before another rescue. Also requires 2 seconds clear conditions.
    // @Units: s
    // @Range: 1 60
    // @User: Advanced
    AP_GROUPINFO("REARM_S", 15, Rescue, rearm_s, 10),
    // @Param: SINK
    // @DisplayName: Dangerous descent rate
    // @Description: Minimum sink for the additional low-height predicted-impact entry path. Requires a confirmed descent and TTC_S margin; not a sink-only trigger.
    // @Units: m/s
    // @Range: 2 20
    AP_GROUPINFO("SINK", 16, Rescue, sink_rate, 5),
    // @Param: UN_ANG
    // @DisplayName: Unstable attitude threshold
    // @Description: Absolute roll or pitch beyond this angle indicates instability during descent and recovery. Attitude alone never captures control.
    // @Units: deg
    // @Range: 30 85
    AP_GROUPINFO("UN_ANG", 17, Rescue, unstable_angle, 60),
    // @Param: UN_RATE
    // @DisplayName: Unstable body rotation rate
    // @Description: Body rotation above this rate detects tumbling during descent and recovery even as attitude passes through level. Rotation alone never captures control.
    // @Units: deg/s
    // @Range: 30 360
    AP_GROUPINFO("UN_RATE", 18, Rescue, unstable_rate, 90),
    // @Param: ST_RATE
    // @DisplayName: Stable body rotation limit
    // @Description: Maximum body rotation for declaring continuous stable flight. A rolling aircraft is never stable just because roll passes zero.
    // @Units: deg/s
    // @Range: 3 30
    AP_GROUPINFO("ST_RATE", 19, Rescue, stable_rate, 15),
    // @Param: PROG_S
    // @DisplayName: Recovery progress window
    // @Description: Recovery must gain PROG_M metres of height or equivalent specific-energy height in each window below the target band. Checked after a 2 second capture grace.
    // @Units: s
    // @Range: 3 30
    AP_GROUPINFO("PROG_S", 20, Rescue, progress_s, 10),
    // @Param: PROG_M
    // @DisplayName: Minimum recovery progress
    // @Description: Minimum height or specific-energy-height gain per progress window below the target band. Airspeed recovery counts as progress; throttle position alone does not. Existing descent and impact limits still apply.
    // @Units: m
    // @Range: 0.5 20
    AP_GROUPINFO("PROG_M", 21, Rescue, progress_m, 3),
    // @Param: IMPACT_S
    // @DisplayName: Emergency ground-datum crossing time
    // @Description: Deploy during recovery when height above Home divided by down speed is below this value and descent exceeds 2m/s. A constant-speed prediction, not guaranteed terrain clearance.
    // @Units: s
    // @Range: 0.5 5
    AP_GROUPINFO("IMPACT_S", 22, Rescue, impact_s, 2),
    // @Param: STK_EN
    // @DisplayName: Accept deliberate stick handover
    // @Description: In the stabilized window a fresh sustained stick or throttle change requests FBWA. Zero retains explicit flight-mode-change handover only.
    // @Values: 0:Mode change only,1:Mode change or deliberate input
    AP_GROUPINFO("STK_EN", 23, Rescue, stick_enable, 1),
    // @Param: STK_US
    // @DisplayName: Pilot input change threshold
    // @Description: PWM change from the snapshot taken at stabilization. Rejects fixed held inputs and small joystick noise.
    // @Units: us
    // @Range: 50 300
    AP_GROUPINFO("STK_US", 24, Rescue, stick_us, 120),
    // @Param: STK_S
    // @DisplayName: Deliberate pilot input dwell
    // @Description: Changed input must persist for this time with fresh valid RC data. Applies equally to receiver, RC overrides and MANUAL_CONTROL.
    // @Units: s
    // @Range: 0.1 1
    AP_GROUPINFO("STK_S", 25, Rescue, stick_s, 0.3),
    // @Param: TTC_S
    // @DisplayName: Low-height predicted-impact entry time
    // @Description: Additional entry path below ALT_TRIG: confirmed descent faster than SINK and predicted Home crossing inside this time, sustained for DWELL_S. Verified braking with stopping clearance suppresses this path. An extension to the acceleration-based brief, not terrain sensing.
    // @Units: s
    // @Range: 3 30
    AP_GROUPINFO("TTC_S", 26, Rescue, crossing_s, 6),
    AP_GROUPEND
};

Rescue::Rescue()
{
    AP_Param::setup_object_defaults(this, var_info);
}

bool Rescue::configuration_ok() const
{
    const float values[] = {alt_warn, alt_trig, acc_z, hand_s, dwell_s, alt_hyst,
                            stable_s, alt_tol, vz_tol, angle_max, recovery_s,
                            fail_s, min_alt, rearm_s, sink_rate, unstable_angle,
                            unstable_rate, stable_rate, progress_s, progress_m,
                            impact_s, stick_us, stick_s, crossing_s};
    for (const float v : values) {
        if (!isfinite(v) || v <= 0) {
            return false;
        }
    }
    return alt_warn <= 10000 && alt_warn > alt_trig && alt_trig > min_alt &&
           alt_tol < alt_warn - alt_trig && alt_tol <= 20 && alt_hyst <= 100 &&
           hand_s >= 0.5f && hand_s <= 60 && dwell_s >= 0.02f && dwell_s <= 2 &&
           stable_s >= 0.2f && stable_s <= 10 && recovery_s >= 5 && recovery_s <= 180 &&
           fail_s >= 0.5f && fail_s <= 10 && rearm_s >= 1 && rearm_s <= 60 &&
           angle_max <= 20 && vz_tol <= 3 && sink_rate >= 2 && sink_rate <= 20 &&
           unstable_angle >= 30 && unstable_angle <= 85 && unstable_rate >= 30 && unstable_rate <= 360 &&
           stable_rate >= 3 && stable_rate <= 30 && progress_s >= 3 && progress_s <= 30 &&
           progress_m >= 0.5f && progress_m <= 20 && impact_s >= 0.5f && impact_s <= 5 &&
           stick_us >= 50 && stick_us <= 300 && stick_s >= 0.1f && stick_s <= 1 &&
           crossing_s >= 3 && crossing_s <= 30 && crossing_s > impact_s;
}

bool Rescue::pre_arm_checks(char *buffer, size_t buflen) const
{
    if (!enabled()) {
        return true;
    }
    if (!configuration_ok()) {
        hal.util->snprintf(buffer, buflen, "invalid rescue parameters");
        return false;
    }
#if HAL_PARACHUTE_ENABLED
    if (!plane.parachute.enabled()) {
        hal.util->snprintf(buffer, buflen, "rescue needs enabled parachute");
        return false;
    }
    return plane.parachute.arming_checks(buflen, buffer);
#else
    hal.util->snprintf(buffer, buflen, "rescue needs parachute support");
    return false;
#endif
}

bool Rescue::sample()
{
    const uint32_t now = AP_HAL::millis();
    // Automatic mode entry calls start() inside this supervisor tick. Reuse
    // that sample: filtering the same acceleration twice changes the trigger
    // value between the WARNING decision and its CAPTURE log record.
    if (have_sample && sample_ms == now) {
        return true;
    }
    Location loc;
    int32_t alt_cm;
    Vector3f vel;
    if (!plane.ahrs.get_location(loc) ||
        !loc.get_alt_cm(Location::AltFrame::ABOVE_HOME, alt_cm) ||
        !plane.ahrs.get_velocity_NED(vel)) {
        descent_since_ms = 0;
        descent_loss = 0;
        have_sample = have_sink_trend = false;
        braking_grace_ms = 0; braking_grace_used = false;
        return false;
    }
    // AHRS earth-frame accelerometer is specific force. Add gravity in NED.
    const float raw_down = plane.ahrs.get_accel_ef().z + GRAVITY_MSS;
    if (!isfinite(raw_down) || !isfinite(vel.z)) {
        descent_since_ms = 0;
        descent_loss = 0;
        have_sample = have_sink_trend = false;
        braking_grace_ms = 0; braking_grace_used = false;
        return false;
    }
    // Check actual down-speed change as well as the accelerometer before
    // trusting a pull-out. One negative acceleration spike is not braking.
    const uint32_t elapsed = now - sample_ms;
    if (have_sample && elapsed > 0 && elapsed <= 100) {
        const float dt = elapsed * 0.001f;
        const float derivative = (vel.z - velocity_down) / dt;
        const float alpha = dt / (0.15915f + dt); // 1 Hz velocity trend
        sink_accel = have_sink_trend ? sink_accel + alpha * (derivative - sink_accel) : derivative;
        have_sink_trend = true;
    } else {
        sink_accel = 0;
        have_sink_trend = false;
        descent_since_ms = 0;
        descent_loss = 0;
        braking_grace_ms = 0; braking_grace_used = false;
    }
    altitude = alt_cm * 0.01f;
    velocity_down = vel.z;
    accel_down = have_sample ? accel_down + 0.38587f * (raw_down - accel_down) : raw_down;
    have_sample = true;
    sample_ms = now;
    airspeed = 0;
    plane.ahrs.airspeed_estimate(airspeed);
    body_rate = degrees(plane.ahrs.get_gyro().length());
    impact_time = velocity_down > 0.5f ? MAX(0.0f, altitude) / velocity_down : 999;
    // During takeoff level-off, downward acceleration first brakes a climb.
    // Even a brief reversal into sink is normal. Require a measured descent
    // trend, rather than letting the first few centimetres capture the mode.
    // Do not exempt TAKEOFF: an actual failed departure still needs recovery.
    if (velocity_down > 0.5f) {
        if (descent_since_ms == 0) {
            descent_since_ms = now;
            descent_peak_alt = altitude;
        }
        descent_peak_alt = MAX(descent_peak_alt, altitude);
        descent_loss = MAX(0.0f, descent_peak_alt - altitude);
    } else {
        descent_since_ms = 0;
        descent_loss = 0;
        braking_grace_ms = 0; braking_grace_used = false;
    }
    const bool confirmed_descent = descent_since_ms != 0 &&
                                   now - descent_since_ms >= 1000 && descent_loss >= 2.0f;
    const bool urgent_crossing = impact_time < impact_s && velocity_down > 2.0f;
    risk = (confirmed_descent && accel_down > acc_z ? 1U : 0U) |
           (confirmed_descent && velocity_down > sink_rate ? 2U : 0U) |
           (unstable_flight() && velocity_down > 2.0f ? 4U : 0U) |
           (body_rate > unstable_rate && velocity_down > 2.0f ? 8U : 0U) |
           (impact_time < crossing_s && velocity_down > 2.0f &&
            (confirmed_descent || urgent_crossing) ? 16U : 0U);
    // A steady dive can have nearly zero acceleration. Keep the brief's
    // acceleration path, but do not ignore a sustained approach to the datum.
    // Ordinary descents have ample time; a demonstrated pull-out gets room.
    const float braking = MIN(-accel_down, -sink_accel);
    const bool braking_clear = velocity_down > 0.5f && have_sink_trend && braking > 1.0f &&
                               altitude - sq(velocity_down) / (2.0f * braking) > 5.0f;
    if (braking_clear) {
        risk |= 64U; // Recorded evidence for yielding to a real pull-out.
    }
    const bool hazardous_sink = velocity_down > sink_rate ||
                                (altitude < min_alt && velocity_down > 2.0f);
    const Vector3f gyro = plane.ahrs.get_gyro();
    const float roll_rad = radians(plane.ahrs.roll_sensor * 0.01f);
    const float pitch_rate_up = degrees(gyro.y * cosf(roll_rad) - gyro.z * sinf(roll_rad));
    const bool pilot_pitch_response = rc().has_valid_input() && now - rc().last_input_ms() <= 250 &&
                                      (plane.control_mode == &plane.mode_fbwa || plane.control_mode == &plane.mode_manual) &&
                                      plane.channel_pitch->norm_input() > 0.2f && pitch_rate_up > 0.5f && accel_down < 0.5f;
    const bool measured_pullout = have_sink_trend && accel_down < -0.5f && sink_accel < -0.25f && pitch_rate_up > 5.0f;
    const bool starting_pullout = (measured_pullout || pilot_pitch_response) &&
                                  altitude > min_alt && impact_time > impact_s;
    // Give a physically responding pull-out half a second to establish its
    // stopping margin. Only once per descent: held sticks or repeated pulses
    // must not keep extending the deadline toward the ground.
    if (confirmed_descent && altitude < alt_trig && hazardous_sink && impact_time < crossing_s &&
        starting_pullout && !braking_grace_used) {
        braking_grace_ms = now;
        braking_grace_used = true;
    }
    const bool pullout_grace = starting_pullout && braking_grace_ms != 0 && now - braking_grace_ms < 500;
    if (pullout_grace) {
        risk |= 128U;
    }
    if (confirmed_descent && altitude < alt_trig && hazardous_sink &&
        impact_time < crossing_s && !braking_clear && !pullout_grace) {
        risk |= 32U;
    }
    return true;
}

bool Rescue::unstable_flight() const
{
    return abs(plane.ahrs.roll_sensor) > unstable_angle * 100 ||
           abs(plane.ahrs.pitch_sensor) > unstable_angle * 100 || body_rate > unstable_rate;
}

bool Rescue::stable_flight() const
{
    return fabsf(altitude - alt_warn) <= alt_tol && fabsf(velocity_down) <= vz_tol &&
           abs(plane.ahrs.roll_sensor) <= angle_max * 100 &&
           abs(plane.ahrs.pitch_sensor) <= angle_max * 100 && body_rate <= stable_rate &&
           airspeed >= plane.aparm.airspeed_min;
}

void Rescue::reset_input_baseline()
{
    const RC_Channel *channels[] = {plane.channel_roll, plane.channel_pitch,
                                    plane.channel_throttle, plane.channel_rudder};
    for (uint8_t i = 0; i < 4; i++) {
        input_baseline[i] = channels[i]->get_radio_in();
        input_overrides[i] = channels[i]->has_override();
    }
    input_since_ms = 0;
}

bool Rescue::pilot_input()
{
    const uint32_t now = AP_HAL::millis();
    if (!stick_enable || !rc().has_valid_input() || now - rc().last_input_ms() > 250) {
        input_since_ms = 0;
        return false;
    }
    const RC_Channel *channels[] = {plane.channel_roll, plane.channel_pitch,
                                    plane.channel_throttle, plane.channel_rudder};
    bool changed = false;
    for (uint8_t i = 0; i < 4; i++) {
        const uint16_t value = channels[i]->get_radio_in();
        if (input_overrides[i] && !channels[i]->has_override()) {
            // Expiry can move a channel without anyone touching a stick.
            // Re-sample the baseline instead of acknowledging that movement.
            reset_input_baseline();
            return false;
        }
        input_overrides[i] = channels[i]->has_override();
        if (value < 900 || value > 2100 || input_baseline[i] < 900 || input_baseline[i] > 2100) {
            reset_input_baseline();
            return false;
        }
        const bool fresh = !channels[i]->has_override() || now - channels[i]->override_timestamp_ms() <= 250;
        changed |= fresh && abs(int32_t(value) - input_baseline[i]) >= stick_us;
    }
    if (!changed) {
        input_since_ms = 0;
    } else if (input_since_ms == 0) {
        input_since_ms = now;
    } else if (now - input_since_ms >= uint32_t(stick_s * 1000)) {
        log(Event::PILOT_INPUT);
        if (plane.set_mode(plane.mode_fbwa, ModeReason::RC_COMMAND)) {
            GCS_SEND_TEXT(MAV_SEVERITY_NOTICE, "RESCUE: pilot input accepted; FBWA control");
            return true;
        }
        input_since_ms = now;
    }
    return false;
}

void Rescue::send_status(mavlink_channel_t chan) const
{
    if (!enabled()) {
        return;
    }
    const uint32_t now = AP_HAL::millis();
    static const char names[][10] = {"RSC_STATE", "RSC_RISK", "RSC_AZ", "RSC_VZ", "RSC_RATE", "RSC_GAIN", "RSC_TTC", "RSC_DLOSS", "RSC_DTIME"};
    const float values[] = {float(uint8_t(state)), float(risk), accel_down, velocity_down,
                            body_rate, altitude - progress_alt, impact_time, descent_loss,
                            descent_since_ms == 0 ? 0.0f : (now - descent_since_ms) * 0.001f};
    for (uint8_t i = 0; i < ARRAY_SIZE(values); i++) {
        mavlink_msg_named_value_float_send(chan, now, names[i], values[i]);
    }
}

void Rescue::log(Event event)
{
#if HAL_LOGGING_ENABLED
    // RSC: state plus the actual detection inputs, and event reason.
    const log_Rescue pkt {
        LOG_PACKET_HEADER_INIT(LOG_RSC_MSG),
        time_us : AP_HAL::micros64(),
        state : uint8_t(state),
        event : uint8_t(event),
        altitude : altitude,
        accel_down : accel_down,
        velocity_down : velocity_down,
        roll : plane.ahrs.roll_sensor * 0.01f,
        pitch : plane.ahrs.pitch_sensor * 0.01f,
        airspeed : airspeed,
        rate : body_rate,
        throttle : SRV_Channels::get_output_scaled(SRV_Channel::k_throttle),
        gain : altitude - progress_alt,
        impact_time : impact_time,
        risk : risk
    };
    if (event == Event::SAMPLE) {
        plane.logger.WriteBlock(&pkt, sizeof(pkt));
    } else {
        // State changes must survive ordinary logging buffer pressure.
        plane.logger.WriteCriticalBlock(&pkt, sizeof(pkt));
    }
#endif
}

void Rescue::transition(State next, Event event)
{
    if (next == state) {
        return;
    }
    state = next;
    log(event);
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "RESCUE state %u event %u", uint8_t(state), uint8_t(event));
}

bool Rescue::start()
{
    if (!enabled() || !configuration_ok() || !plane.arming.is_armed() || !plane.is_flying()) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "RESCUE entry rejected: config/armed/flight");
        return false;
    }
#if HAL_PARACHUTE_ENABLED
    char buffer[80];
    if (!plane.parachute.enabled() || !plane.parachute.arming_checks(sizeof(buffer), buffer) ||
        plane.parachute.released() || plane.parachute.release_in_progress()) {
        GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE entry rejected: parachute unavailable");
        return false;
    }
#else
    GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE requires parachute support");
    return false;
#endif
    if (!sample()) {
        return false;
    }
    capture_ms = AP_HAL::millis();
    stable_since_ms = handover_ms = fail_since_ms = clear_since_ms = 0;
    progress_ms = unstable_ms = lost_stable_ms = sensor_lost_ms = input_since_ms = 0;
    progress_alt = altitude;
    progress_speed = airspeed;
    transition(State::RECOVER, Event::CAPTURE);
    GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE: controls captured");
    return true;
}

bool Rescue::allow_mode_change(ModeReason reason) const
{
    // Emergency/failsafe modes may still preempt. Pilot handover is explicit.
    if (reason == ModeReason::RC_COMMAND || reason == ModeReason::GCS_COMMAND ||
        reason == ModeReason::AUX_FUNCTION) {
        return state == State::STABILIZED &&
               AP_HAL::millis() - handover_ms < uint32_t(hand_s * 1000);
    }
    return state != State::CHUTE;
}

void Rescue::stop()
{
    if (state == State::CHUTE) {
        return;
    }
    cooldown_ms = AP_HAL::millis();
    clear_since_ms = fall_since_ms = stable_since_ms = 0;
    transition(State::COOLDOWN, enabled() ? Event::HANDOVER : Event::DISABLED);
}

void Rescue::deploy(Event event)
{
    if (state == State::CHUTE) {
        return;
    }
#if HAL_PARACHUTE_ENABLED
    if (!plane.parachute.enabled()) {
        GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE: parachute disabled during recovery");
        return; // Do not stop a flyable aircraft without a usable parachute.
    }
    transition(State::CHUTE, event);
    GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE: deploying parachute");
    plane.parachute_release();
    // AP_Parachute normally disarms before activating release, to protect the canopy.
    SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, 0);
    plane.arming.disarm(AP_Arming::Method::PARACHUTE_RELEASE, false);
    plane.throttle_suppressed = true;
#endif
}

void Rescue::update()
{
    const uint32_t now = AP_HAL::millis();
    if (!enabled()) {
        if (plane.control_mode == &plane.mode_rescue && state != State::CHUTE) {
            plane.set_mode(*plane.previous_mode, ModeReason::RESCUE_DISABLED);
        }
        state = State::MONITOR;
        have_sample = false;
        have_sink_trend = false;
        descent_since_ms = 0;
        descent_loss = 0;
        fall_since_ms = clear_since_ms = 0;
        return;
    }
    if (state == State::CHUTE) {
        return; // Terminal until reboot; the parachute cannot be repacked in flight.
    }
    if (!plane.arming.is_armed()) {
        state = State::MONITOR;
        fall_since_ms = clear_since_ms = 0;
        have_sample = false;
        have_sink_trend = false;
        descent_since_ms = 0;
        descent_loss = 0;
        return;
    }
    if (!configuration_ok()) {
        if (!config_warned) {
            GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE: invalid parameters");
            config_warned = true;
        }
        // Keep flying under the last recovery controller; its watchdog still runs.
        if (plane.control_mode == &plane.mode_rescue) {
            deploy(Event::BAD_CONFIG);
        }
        return;
    }
    config_warned = false;
    if (!sample()) {
        if (plane.control_mode == &plane.mode_rescue) {
            if (sensor_lost_ms == 0) { sensor_lost_ms = now; }
            if (now - sensor_lost_ms > 2000) { deploy(Event::SENSOR_LOSS); }
        }
        return;
    }
    sensor_lost_ms = 0;
    if (now - log_ms >= 100) {
        log(Event::SAMPLE);
        log_ms = now;
    }
    if (plane.control_mode == &plane.mode_rescue) {
        if (altitude < min_alt && velocity_down > 0.5f) {
            deploy(Event::LOW_ALT);
            return;
        }
        if (impact_time < impact_s && velocity_down > 2.0f) {
            deploy(Event::IMPACT_RISK);
            return;
        }
        if (state == State::STABILIZED) {
            if (!stable_flight()) {
                input_since_ms = 0;
                if (lost_stable_ms == 0) { lost_stable_ms = now; }
                if (now - lost_stable_ms >= 500) {
                    stable_since_ms = handover_ms = progress_ms = 0;
                    transition(State::RECOVER, Event::STABILITY_LOST);
                    GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "RESCUE: stability lost; handover cancelled");
                }
                return;
            }
            lost_stable_ms = 0;
            if (pilot_input()) { return; }
            if (now - handover_ms >= uint32_t(hand_s * 1000)) {
                deploy(Event::NO_PILOT);
            }
            return;
        }
        if (now - capture_ms > 2000 && unstable_flight()) {
            if (unstable_ms == 0) { unstable_ms = now; }
            else if (now - unstable_ms >= uint32_t(fail_s * 1000)) { deploy(Event::UNSTABLE); return; }
        } else { unstable_ms = 0; }
        if (now - capture_ms >= uint32_t(recovery_s * 1000)) {
            deploy(Event::TIMEOUT);
            return;
        }
        if (now - capture_ms > 2000 && velocity_down > 3.0f) {
            if (fail_since_ms == 0) {
                fail_since_ms = now;
            } else if (now - fail_since_ms >= uint32_t(fail_s * 1000)) {
                deploy(Event::NO_CLIMB);
                return;
            }
        } else {
            fail_since_ms = 0;
        }
        if (now - capture_ms > 2000 && altitude < alt_warn - 2 * alt_tol) {
            if (progress_ms == 0) { progress_ms = now; progress_alt = altitude; progress_speed = airspeed; }
            else if (now - progress_ms >= uint32_t(progress_s * 1000)) {
                // Building airspeed is useful progress even before a climb.
                // TECS needs that option when the aircraft is short of energy.
                const float height_gain = altitude - progress_alt;
                const float energy_gain = height_gain + (sq(airspeed) - sq(progress_speed)) / (2 * GRAVITY_MSS);
                if (height_gain < progress_m && energy_gain < progress_m) { deploy(Event::NO_PROGRESS); return; }
                if (height_gain < progress_m) {
                    log(Event::ENERGY_PROGRESS);
                    GCS_SEND_TEXT(MAV_SEVERITY_NOTICE, "RESCUE: airspeed recovery counts as progress");
                }
                progress_ms = now; progress_alt = altitude; progress_speed = airspeed;
            }
        } else { progress_ms = 0; }
        const bool stable = stable_flight();
        if (stable) {
            if (stable_since_ms == 0) {
                stable_since_ms = now;
            } else if (now - stable_since_ms >= uint32_t(stable_s * 1000)) {
                handover_ms = now;
                reset_input_baseline();
                transition(State::STABILIZED, Event::STABLE);
                GCS_SEND_TEXT(MAV_SEVERITY_NOTICE, "RESCUE: aircraft stabilized; take control");
            }
        } else {
            stable_since_ms = 0;
        }
        return;
    }
    if (!plane.is_flying()) {
        fall_since_ms = clear_since_ms = 0;
        descent_since_ms = 0;
        descent_loss = 0;
        return;
    }
    const bool clear = altitude >= alt_warn + alt_hyst ||
                       ((risk & 33U) == 0 && velocity_down < 0.5f);
    if (state == State::COOLDOWN) {
        // Handover hysteresis must not create a blind interval in which a new
        // dive can hit the ground. Only a persistent imminent low-height risk
        // overrides cooldown; ordinary level flight / gentle descent cannot.
        const bool imminent = (risk & 32U) != 0 ||
                              ((risk & 1U) != 0 && altitude < alt_trig && velocity_down > 2.0f &&
                               (altitude < min_alt || impact_time < impact_s));
        if (imminent) {
            if (fall_since_ms == 0) { fall_since_ms = now; }
            else if (now - fall_since_ms >= uint32_t(dwell_s * 1000)) {
                GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE: immediate danger overrides cooldown");
                plane.set_mode(plane.mode_rescue, ModeReason::RESCUE);
                fall_since_ms = 0;
                return;
            }
        } else { fall_since_ms = 0; }
        if (clear) {
            if (clear_since_ms == 0) {
                clear_since_ms = now;
            }
            if (now - cooldown_ms >= uint32_t(rearm_s * 1000) && now - clear_since_ms >= 2000) {
                transition(State::MONITOR, Event::CLEAR);
            }
        } else {
            clear_since_ms = 0;
        }
        return;
    }
    // Bit 1 is the requested acceleration rule; bit 32 is the separate
    // low-height steady-descent safeguard. Clear both on recovery so a later
    // climb cannot inherit an old fall qualification.
    const bool falling = altitude < alt_warn && (risk & 33U) != 0;
    if (falling) {
        clear_since_ms = 0;
        if (fall_since_ms == 0) {
            fall_since_ms = now;
        }
        if (now - fall_since_ms >= uint32_t(dwell_s * 1000)) {
            transition(State::WARNING, Event::FALL);
            if (warning_ms == 0 || now - warning_ms >= 1000) {
                GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "RESCUE WARNING: h%.0f sink%.1f risk%u", double(altitude), double(velocity_down), risk);
                warning_ms = now;
            }
            if (altitude < alt_trig) {
                if (risk & 32U) {
                    GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "RESCUE: sustained sink; predicted impact");
                }
                if (!plane.set_mode(plane.mode_rescue, ModeReason::RESCUE)) {
                    // Without a usable parachute do not substitute an unsafe motor cut.
                    transition(State::COOLDOWN, Event::BAD_CONFIG);
                    cooldown_ms = now;
                }
            }
        }
    } else {
        fall_since_ms = 0;
        // Accelerating descent can end before height returns above warning altitude.
        const bool warning_clear = altitude >= alt_warn + alt_hyst || (risk & 33U) == 0;
        if (state == State::WARNING && warning_clear) {
            if (clear_since_ms == 0) {
                clear_since_ms = now;
            } else if (now - clear_since_ms >= 500) {
                transition(State::MONITOR, Event::CLEAR);
                warning_ms = 0;
            }
        } else {
            clear_since_ms = 0;
        }
    }
}

void Plane::rescue_update()
{
    // With RSC_ENABLE=0 and stock modes this is a single inactive branch.
    if (g2.rescue.enabled() || control_mode == &mode_rescue) {
        g2.rescue.update();
    }
}
