// RESCUE mode integration: Saeed Nik.
#include "mode.h"
#include "Plane.h"

bool ModeRescue::_enter()
{
    if (!plane.g2.rescue.start()) {
        return false;
    }
    reset_controllers();
    // Re-enter the existing TECS loop from a usable command, even if capture
    // happens almost vertically. Its normal speed and acceleration limits stay on.
    plane.TECS_controller.reset(true);
    plane.inverted_flight = false;
    plane.guided_throttle_passthru = false;
    plane.auto_state.takeoff_complete = true;
    update_target_altitude();
    return true;
}

void ModeRescue::_exit()
{
    plane.g2.rescue.stop();
}

void ModeRescue::update_target_altitude()
{
    Location target = plane.current_loc;
    target.set_alt_cm(plane.g2.rescue.target_altitude() * 100,
                      Location::AltFrame::ABOVE_HOME);
    plane.set_target_altitude_location(target);
    plane.reset_offset_altitude();
}

void ModeRescue::update()
{
    // Set targets here and leave pitch/throttle demand to TECS. A second
    // recovery controller would compete with its speed protection.
    update_target_altitude();
    plane.nav_roll_cd = 0;
    plane.update_load_factor();
    plane.calc_nav_pitch();
    plane.calc_throttle();
    if (plane.g2.rescue.chute_requested()) {
        SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, 0);
    }
}

void ModeRescue::run()
{
    // No pilot stick mixing while recovery owns the flight controls.
    plane.stabilize_roll();
    plane.stabilize_pitch();
    plane.stabilize_yaw();
}
