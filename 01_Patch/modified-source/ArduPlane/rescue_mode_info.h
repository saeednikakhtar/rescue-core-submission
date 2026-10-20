// GPLv3 or later, like ArduPlane.
// Backport the outbound MAVLink AVAILABLE_MODES wire format to the pinned
// 4.6.3 dialect. This changes metadata only; mode numbers/control stay intact.
// Wire layout/CRC: mavlink/c_library_v2 common/mavlink_msg_available_modes.h
// and common/mavlink_msg_available_modes_monitor.h (official generated headers).
#pragma once

static constexpr uint32_t rescue_available_modes_id = 435;

static inline void rescue_send_mode_info(mavlink_channel_t channel, uint8_t count,
                                        uint8_t index, uint32_t mode_number,
                                        bool user_selectable, const char *name)
{
    char payload[46] {};
    _mav_put_uint32_t(payload, 0, mode_number);
    // MAV_MODE_PROPERTY_NOT_USER_SELECTABLE = 2. NON_STANDARD mode = 0.
    _mav_put_uint32_t(payload, 4, user_selectable ? 0 : 2);
    _mav_put_uint8_t(payload, 8, count);
    _mav_put_uint8_t(payload, 9, index);
    // Zero initialization supplies standard_mode=0 and a terminated name.
    strncpy(&payload[11], name, 34);
    _mav_finalize_message_chan_send(channel, rescue_available_modes_id,
                                   payload, 46, 46, 134);
}

static inline void rescue_send_mode_monitor(mavlink_channel_t channel)
{
    const char sequence = 1;
    _mav_finalize_message_chan_send(channel, 437, &sequence, 1, 1, 30);
}
