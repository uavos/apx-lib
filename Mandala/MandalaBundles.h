/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>
#include <sys/types.h>

namespace mandala {
namespace bundle {

#pragma pack(1)

enum {
    acc_immediate = 1,
    acc_delta = 2,
};
struct acc_delta_s
{
    uint32_t dt;
    float x;
    float y;
    float z;
    float temp;
    float vib;
    uint8_t clipping[3];
};
struct acc_immediate_s
{
    uint32_t dt;
    float x;
    float y;
    float z;
};

enum {
    gyro_immediate = 1,
    gyro_delta = 2,
};
struct gyro_delta_s
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
struct gyro_immediate_s
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
    int32_t llh[3]; // 1e-7 degrees, 1e-3 meters
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
    float att[3];

    float acc[3];
    float gyro[3];

    float llh[3];
    float vel[3];

    float agl;
    float rpm;

    float airspeed;
    float baro_mbar;

    float room_temp;
    float air_temp;
};

#pragma pack()

} // namespace bundle
} // namespace mandala
