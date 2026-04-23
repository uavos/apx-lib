/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cinttypes>
#include <sys/types.h>

namespace mandala {
namespace bundle {

#pragma pack(1)

enum {
    imu_immediate = 1,
    imu_delta = 2,
};
struct imu_delta_s
{
    uint32_t dt;
    float x;
    float y;
    float z;
    float temp;
    float vib;
    float coning;
    uint8_t clipping[3];
};
struct imu_immediate_s
{
    uint32_t dt;
    float x;
    float y;
    float z;
};

struct mag_s
{
    float x;
    float y;
    float z;
    float temp;
    float vib;
};

enum gps_status_e {
    gps_status = 1,
    gps_fix = 2,
    gps_yaw = 4,
};
enum gps_emi_e {
    emi_unknown = 0,
    emi_ok,
    emi_warning,
    emi_critical,
};
struct gps_status_s
{
    uint32_t time;
    enum gps_fix_e {
        fix_none = 0,
        fix_2D = 1,
        fix_3D = 2,
        fix_DIFF = 3,
        fix_RTK = 4,
    } fix : 8; // reflects mandala::gps_fix_*
    enum gps_emi_e {
        emi_unknown = 0, // no EMI detection available
        emi_ok,          // no EMI detected
        emi_warning,     // EMI detected, but fix ok
        emi_critical,    // EMI detected, fix is not reliable
    } emi;
    uint8_t su;
    uint8_t sv;
};
struct gps_fix_s
{
    int32_t llh[3]; // [gps] 1e-7 degrees, 1e-3 meters
    int16_t vel[3]; // 1e-2 m/s

    uint16_t hacc; // mm
    uint16_t vacc; // mm
    uint16_t sacc; // cm/s

    uint16_t gspeed; // cm/s
    uint16_t pdop;   // 0.01 (1=ideal, >20=poor)
};
struct gps_yaw_s
{
    float yaw;
    float yaw_offset;
};

struct vps_s // Visual Positioning System
{
    float pos[3]; // [m] Local position
    float vel[3]; // [m/s] Local velocity

    float pos_var[3]; // [m^2] Position variance
    float vel_var[3]; // [m^2/s^2] Velocity variance
};

struct baro_s
{
    float mbar;
    float temp;
};

struct pitot_s
{
    float pa;
    float temp;
    float eas_scale;
};

struct sim_s
{
    float att_deg[3];  // degrees
    float gyro_deg[3]; // deg/s
    float acc_ned[3];  // NED

    double lat_deg;
    double lon_deg;
    float hmsl;

    float vel_ned[3];

    float agl;
    float rpm;

    float airspeed;
    float baro_mbar;

    float room_temp;
    float air_temp;

    float slip; //degrees
};

struct pos_ll_s
{
    uint32_t lat; // [gps] 1e-7 degrees
    uint32_t lon; // [gps] 1e-7 degrees
};

struct pos_llh_s : public pos_ll_s
{
    float hmsl; // [gps] m
};

enum {
    swarm_followme = 1,
    swarm_data = 2,
};

union swarmid_s {
    uint16_t raw;
    struct
    {
        uint16_t src : 11;
        uint8_t type : 4;
        uint8_t ext : 1;
    };
};

struct swarm_s
{
    swarmid_s id;
    float cmd_airspeed; // m/s
    float cmd_altitude; // m
    float cmd_bearing;  // rad
    float cmd_lacc;     // m/s^2
    float bearing;      // rad
    int32_t cmd_llh[3]; // 1e-7 degrees, 1e-3 meters
};

union ekf_filter_control_status_low_u {
    struct
    {
        bool tilt_align : 1;              ///< 0 - true if the filter tilt alignment is complete
        bool yaw_align : 1;               ///< 1 - true if the filter yaw alignment is complete
        bool gnss_pos : 1;                ///< 2 - true if GNSS position measurement fusion is intended
        bool opt_flow : 1;                ///< 3 - true if optical flow measurements fusion is intended
        bool mag_hdg : 1;                 ///< 4 - true if a simple magnetic yaw heading fusion is intended
        bool mag_3D : 1;                  ///< 5 - true if 3-axis magnetometer measurement fusion is intended
        bool mag_dec : 1;                 ///< 6 - true if synthetic magnetic declination measurements fusion is intended
        bool in_air : 1;                  ///< 7 - true when the vehicle is airborne
        bool wind : 1;                    ///< 8 - true when wind velocity is being estimated
        bool baro_hgt : 1;                ///< 9 - true when baro data is being fused
        bool rng_hgt : 1;                 ///< 10 - true when range finder data is being fused for height aiding
        bool gps_hgt : 1;                 ///< 11 - true when GPS altitude is being fused
        bool ev_pos : 1;                  ///< 12 - true when local position data fusion from external vision is intended
        bool ev_yaw : 1;                  ///< 13 - true when yaw data from external vision measurements fusion is intended
        bool ev_hgt : 1;                  ///< 14 - true when height data from external vision measurements is being fused
        bool fuse_beta : 1;               ///< 15 - true when synthetic sideslip measurements are being fused
        bool mag_field_disturbed : 1;     ///< 16 - true when the mag field does not match the expected strength
        bool fixed_wing : 1;              ///< 17 - true when the vehicle is operating as a fixed wing vehicle
        bool mag_fault : 1;               ///< 18 - true when the magnetometer has been declared faulty and is no longer being used
        bool fuse_aspd : 1;               ///< 19 - true when airspeed measurements are being fused
        bool gnd_effect : 1;              ///< 20 - true when protection from ground effect induced static pressure rise is active
        bool rng_stuck : 1;               ///< 21 - true when rng data wasn't ready for more than 10s and new rng values haven't changed enough
        bool gnss_yaw : 1;                ///< 22 - true when yaw (not ground course) data fusion from a GPS receiver is intended
        bool mag_aligned_in_flight : 1;   ///< 23 - true when the in-flight mag field alignment has been completed
        bool ev_vel : 1;                  ///< 24 - true when local frame velocity data fusion from external vision measurements is intended
        bool synthetic_mag_z : 1;         ///< 25 - true when we are using a synthesized measurement for the magnetometer Z component
        bool vehicle_at_rest : 1;         ///< 26 - true when the vehicle is at rest
        bool gnss_yaw_fault : 1;          ///< 27 - true when the GNSS heading has been declared faulty and is no longer being used
        bool rng_fault : 1;               ///< 28 - true when the range finder has been declared faulty and is no longer being used
        bool inertial_dead_reckoning : 1; ///< 29 - true if we are no longer fusing measurements that constrain horizontal velocity drift
        bool wind_dead_reckoning : 1;     ///< 30 - true if we are navigationg reliant on wind relative measurements
        bool rng_kin_consistent : 1;      ///< 31 - true when the range finder kinematic consistency check is passing
    } flags;
    uint32_t value;
};

union ekf_filter_control_status_high_u {
    struct
    {
        bool fake_pos : 1;               ///< 0 - true when fake position measurements are being fused
        bool fake_hgt : 1;               ///< 1 - true when fake height measurements are being fused
        bool gravity_vector : 1;         ///< 2 - true when gravity vector measurements are being fused
        bool mag : 1;                    ///< 3 - true if 3-axis magnetometer measurement fusion (mag states only) is intended
        bool ev_yaw_fault : 1;           ///< 4 - true when the EV heading has been declared faulty and is no longer being used
        bool mag_heading_consistent : 1; ///< 5 - true when the heading obtained from mag data is declared consistent with the filter
        bool aux_gpos : 1;               ///< 6 - true if auxiliary global position measurement fusion is intended
        bool rng_terrain : 1;            ///< 7 - true if we are fusing range finder data for terrain
        bool opt_flow_terrain : 1;       ///< 8 - true if we are fusing flow data for terrain
        bool valid_fake_pos : 1;         ///< 9 - true if a valid constant position is being fused
        bool constant_pos : 1;           ///< 10 - true if the vehicle is at a constant position
        bool baro_fault : 1;             ///< 11 - true when the baro has been declared faulty and is no longer being used
        bool gnss_vel : 1;               ///< 12 - true if GNSS velocity measurement fusion is intended
        bool gnss_fault : 1;             ///< 13 - true if GNSS measurements (lat, lon, vel) have been declared faulty and are no longer used
        bool yaw_manual : 1;             ///< 14 - true if yaw has been reset manually
        bool gnss_hgt_fault : 1;         ///< 15 - true if GNSS measurements (alt) have been declared faulty and are no longer used
        bool in_transition_to_fw : 1;    ///< 16 - true if the vehicle is in transition to fw
    } flags;
    uint32_t value;
};

union ekf_fault_status_u {
    struct
    {
        bool bad_mag_x : 1;        ///< 0 - true if the fusion of the magnetometer X-axis has encountered a numerical error
        bool bad_mag_y : 1;        ///< 1 - true if the fusion of the magnetometer Y-axis has encountered a numerical error
        bool bad_mag_z : 1;        ///< 2 - true if the fusion of the magnetometer Z-axis has encountered a numerical error
        bool bad_hdg : 1;          ///< 3 - true if the fusion of the heading angle has encountered a numerical error
        bool bad_mag_decl : 1;     ///< 4 - true if the fusion of the magnetic declination has encountered a numerical error
        bool bad_airspeed : 1;     ///< 5 - true if fusion of the airspeed has encountered a numerical error
        bool bad_sideslip : 1;     ///< 6 - true if fusion of the synthetic sideslip constraint has encountered a numerical error
        bool bad_optflow_X : 1;    ///< 7 - true if fusion of the optical flow X axis has encountered a numerical error
        bool bad_optflow_Y : 1;    ///< 8 - true if fusion of the optical flow Y axis has encountered a numerical error
        bool __UNUSED : 1;         ///< 9 -
        bool bad_acc_vertical : 1; ///< 10 - true if bad vertical accelerometer data has been detected
        bool bad_acc_clipping : 1; ///< 11 - true if delta velocity data contains clipping (asymmetric railing)
    } flags;
    uint16_t value;
};

union ekf_information_event_status_u {
    struct
    {
        bool gps_checks_passed : 1;          ///< 0 - true when gps quality checks are passing passed
        bool reset_vel_to_gps : 1;           ///< 1 - true when the velocity states are reset to the gps measurement
        bool reset_vel_to_flow : 1;          ///< 2 - true when the velocity states are reset using the optical flow measurement
        bool reset_vel_to_vision : 1;        ///< 3 - true when the velocity states are reset to the vision system measurement
        bool reset_vel_to_zero : 1;          ///< 4  - true when the velocity states are reset to zero
        bool reset_pos_to_last_known : 1;    ///< 5 - true when the position states are reset to the last known position
        bool reset_pos_to_gps : 1;           ///< 6 - true when the position states are reset to the gps measurement
        bool reset_pos_to_vision : 1;        ///< 7 - true when the position states are reset to the vision system measurement
        bool starting_gps_fusion : 1;        ///< 8 - true when the filter starts using gps measurements to correct the state estimates
        bool starting_vision_pos_fusion : 1; ///< 9 - true when the filter starts using vision system position measurements to correct the state estimates
        bool starting_vision_vel_fusion : 1; ///< 10 - true when the filter starts using vision system velocity measurements to correct the state estimates
        bool starting_vision_yaw_fusion : 1; ///< 11 - true when the filter starts using vision system yaw  measurements to correct the state estimates
        bool yaw_aligned_to_imu_gps : 1;     ///< 12 - true when the filter resets the yaw to an estimate derived from IMU and GPS data
        bool reset_hgt_to_baro : 1;          ///< 13 - true when the vertical position state is reset to the baro measurement
        bool reset_hgt_to_gps : 1;           ///< 14 - true when the vertical position state is reset to the gps measurement
        bool reset_hgt_to_rng : 1;           ///< 15 - true when the vertical position state is reset to the rng measurement
        bool reset_hgt_to_ev : 1;            ///< 16 - true when the vertical position state is reset to the ev measurement
        bool reset_pos_to_ext_obs : 1;       ///< 17 - true when horizontal position was reset to an external observation while deadreckoning
        bool reset_wind_to_ext_obs : 1;      ///< 18 - true when wind states were reset to an external observation
    } flags;
    uint32_t value;
};

#pragma pack()

typedef uint8_t vcp_id_t;

} // namespace bundle
} // namespace mandala
