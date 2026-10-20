/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
  simple parachute simulator class
*/

#include "SIM_Parachute.h"
#include "AP_HAL/AP_HAL.h"
#include <AP_Math/AP_Math.h>
#include <GCS_MAVLink/GCS.h>

using namespace SITL;

// table of user settable parameters
const AP_Param::GroupInfo Parachute::var_info[] = {

    // @Param: ENABLE
    // @DisplayName: Parachute Sim enable/disable
    // @Description: Allows you to enable (1) or disable (0) the Parachute simulation
    // @Values: 0:Disabled,1:Enabled
    // @User: Advanced
    AP_GROUPINFO("ENABLE", 0, Parachute, parachute_enable, 0),

    // @Param: PIN
    // @DisplayName: Parachute pin
    // @Description: The pin number that the Parachute pyrotechnics are connected to. (start at 1)
    // @Range: 0 15
    // @User: Advanced
    AP_GROUPINFO("PIN", 1, Parachute, parachute_pin, -1),

    // @Param: CD_A
    // @DisplayName: Full canopy effective drag area
    // @Description: Drag coefficient times projected canopy area. Illustrative default for a 2 kg model, not measured aircraft data.
    // @Units: m*m
    // @Range: 0 20
    AP_GROUPINFO("CD_A", 2, Parachute, cd_area, 1.3),

    // @Param: OPEN_S
    // @DisplayName: Canopy inflation time
    // @Description: Time from release to full effective drag area, with smooth progressive inflation.
    // @Units: s
    // @Range: 0.1 10
    AP_GROUPINFO("OPEN_S", 3, Parachute, opening_s, 1.5),

    AP_GROUPEND
};

/*
  update parachute state
 */
void Parachute::update(const struct sitl_input &input)
{
    const int16_t pwm = parachute_pin >= 1 && parachute_pin <= 16 ? input.servos[parachute_pin-1] : -1;
    const uint64_t now = AP_HAL::micros64();
    if (pwm >= 1250) {
        if (!deployed_ms) {
            deployed_ms = AP_HAL::millis();
            GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "BANG!  Parachute deployed");
        }
    }
    if (now - last_report_us >= report_interval) {
        last_report_us = now;
        gcs().send_named_float("SIM_CHUTE", inflation());
    }
}

float Parachute::inflation() const
{
    if (!is_enabled() || deployed_ms == 0) {
        return 0;
    }
    const float elapsed = (AP_HAL::millis() - deployed_ms) * 0.001f;
    const float fraction = constrain_float(elapsed / MAX(float(opening_s), 0.1f), 0, 1);
    return fraction * fraction * (3 - 2 * fraction);
}

Vector3f Parachute::drag(const Vector3f &velocity_air, float density) const
{
    // Quadratic drag opposes all air-relative motion, including crosswind drift.
    const float scale = -0.5f * MAX(density, 0.0f) * MAX(float(cd_area), 0.0f) * inflation() * velocity_air.length();
    return velocity_air * scale;
}
