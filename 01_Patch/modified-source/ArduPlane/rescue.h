#pragma once

#include <AP_Param/AP_Param.h>
#include <AP_Vehicle/ModeReason.h>
#include <AP_Logger/LogStructure.h>

struct PACKED log_Rescue {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    uint8_t state;
    uint8_t event;
    float altitude;
    float accel_down;
    float velocity_down;
    float roll;
    float pitch;
    float airspeed;
    float rate;
    float throttle;
    float gain;
    float impact_time;
    uint16_t risk;
};

// Fixed-wing recovery supervisor. All actuator control remains in ModeRescue/TECS.
class Rescue {
public:
    Rescue();
    static const AP_Param::GroupInfo var_info[];
    enum class State : uint8_t { MONITOR, WARNING, RECOVER, STABILIZED, CHUTE, COOLDOWN };
    enum class Event : uint8_t { SAMPLE, FALL, CLEAR, CAPTURE, STABLE, HANDOVER,
                                 NO_PILOT, LOW_ALT, NO_CLIMB, TIMEOUT, DISABLED, BAD_CONFIG,
                                 PILOT_INPUT, NO_PROGRESS, UNSTABLE, STABILITY_LOST,
                                 IMPACT_RISK, SENSOR_LOSS, ENERGY_PROGRESS };
    void update();
    bool start();
    void stop();
    bool pre_arm_checks(char *buffer, size_t buflen) const;
    bool allow_mode_change(ModeReason reason) const;
    bool enabled() const { return enable.get() > 0; }
    bool climbing() const { return state == State::RECOVER; }
    bool chute_requested() const { return state == State::CHUTE; }
    float target_altitude() const { return alt_warn.get(); }
    float altitude_tolerance() const { return alt_tol.get(); }
    void send_status(mavlink_channel_t chan) const;

private:
    AP_Int8 enable;
    AP_Float alt_warn, alt_trig, acc_z, hand_s;
    AP_Float dwell_s, alt_hyst, stable_s, alt_tol, vz_tol, angle_max;
    AP_Float recovery_s, fail_s, min_alt, rearm_s;
    AP_Float sink_rate, unstable_angle, unstable_rate, stable_rate;
    AP_Float progress_s, progress_m, impact_s, stick_us, stick_s, crossing_s;
    AP_Int8 stick_enable;
    State state = State::MONITOR;
    uint32_t fall_since_ms = 0, clear_since_ms = 0, stable_since_ms = 0;
    uint32_t capture_ms = 0, handover_ms = 0, fail_since_ms = 0;
    uint32_t cooldown_ms = 0, warning_ms = 0, log_ms = 0;
    uint32_t sample_ms = 0;
    uint32_t descent_since_ms = 0;
    uint32_t braking_grace_ms = 0;
    uint32_t progress_ms = 0, unstable_ms = 0, lost_stable_ms = 0;
    uint32_t input_since_ms = 0, sensor_lost_ms = 0;
    uint16_t input_baseline[4] = {};
    bool input_overrides[4] = {};
    uint16_t risk = 0;
    float altitude = 0, accel_down = 0, velocity_down = 0;
    float airspeed = 0, body_rate = 0, progress_alt = 0, progress_speed = 0, impact_time = 999;
    float descent_peak_alt = 0, descent_loss = 0;
    float sink_accel = 0;
    bool have_sink_trend = false;
    bool braking_grace_used = false;
    bool have_sample = false;
    bool config_warned = false;

    bool configuration_ok() const;
    bool sample();
    bool stable_flight() const;
    bool unstable_flight() const;
    void reset_input_baseline();
    bool pilot_input();
    void transition(State next, Event event);
    void log(Event event);
    void deploy(Event event);
};
