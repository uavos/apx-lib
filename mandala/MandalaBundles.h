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

enum {
    gps_status = 1,
    gps_fix = 2,
    gps_yaw = 4,
};
struct gps_status_s
{
    uint32_t time;
    uint8_t status;
    uint8_t emi;
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
    uint32_t lat; // [gps] units
    uint32_t lon; // [gps] units
};

struct pos_llh_s
{
    uint32_t lat; // [gps] units
    uint32_t lon; // [gps] units
    float hmsl;   // [gps] m
};

struct haps_center_s
{
    float roll;
    float left_cmd_roll;
    float right_cmd_roll;
    float err_shape;
    float cmd_pitch;
    float cmd_airspeed;
    float rc_roll;
    float rc_pitch;
    float rc_yaw;
    float ctr_throttle;
    float ctr_rudder;
    uint8_t EMG_MODE : 1;
    uint8_t RESET_PID : 1;
    uint8_t unused : 6;
};

struct haps_lr_s
{
    float roll;
    float pitch;
    float yaw;
    float airspeed;
    float cmd_pitch;
    float ctr_elevator;
    float ctr_haps_ailerons;
};

#pragma pack()

typedef uint8_t vcp_id_t;

} // namespace bundle
} // namespace mandala
