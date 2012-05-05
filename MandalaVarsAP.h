/*
 * Copyright (C) 2011 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 *  http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MANDALA_AP_CONSTANTS
#define MANDALA_AP_CONSTANTS
//=============================================================================
// AP constants
//=============================================================================
#define AHRS_FREQ       100     // AHRS Update (main loop freq) [Hz]
#define GPS_FREQ        5       // GPS Update rate (for derivatives) [Hz]
#define CTR_FREQ        20      // ctr (fast servo) update rate [Hz]
#define TELEMETRY_FREQ  10      // Telemetry send rate [Hz] MAX 10Hz!
#define SIM_FREQ        10      // Simulator servo send rate [Hz]
#define MAX_TELEMETRY   64      // max telemetry packet size [bytes]
//=============================================================================
#endif

//=============================================================================
//    PID Regulators definitions. makes enum and reg_descr[], reg_names[]
// <reg enum name>, <description>
//------------------------------------------------------------------------------
#ifndef REGDEF
#define REGDEF(aname,adescr)
#endif
REGDEF(Roll,    "[PID] Roll angle to Ailerons")
REGDEF(Pitch,   "[PID] Pitch angle to Elevator")
REGDEF(Thr,     "[PI]  Airspeed to Throttle/RPM")
REGDEF(Rpm,     "[PI]  RPM to Throttle")
REGDEF(Course,  "[PID] Course to Roll angle")
REGDEF(Vspeed,  "[PI]  Vertical Speed to Pitch angle")
REGDEF(Altitude,"[P]   Altitude to Vertical Speed")
REGDEF(Taxi,    "[PID] Runway Displacement to Yaw")
REGDEF(Steering,"[PID] Yaw angle to Steering")
REGDEF(Runway,  "[PID] Runway Displacement to Course")
REGDEF(Path,    "[PID] Trajectory Path to Course")
REGDEF(Glider,  "[PI]  Airspeed to Pitch angle")
REGDEF(Slip,    "[PI]  Slip rate to Rudder")
REGDEF(HoverR,  "[PPI] Hovering X Displacement to Roll")
REGDEF(HoverP,  "[PPI] Hovering Y Displacement to Pitch")
#undef REGDEF
//=============================================================================


//=============================================================================
//--------- CONFIG --------------
#ifndef CFGDEF
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)
#endif
#ifndef CFGDEFA
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) CFGDEF(atype,aname,aspan,abytes,around,adescr)
#endif
// description format:
// <Group>: text (vect axis names if any)
// vect axis names used to create text names in ConfigModel
// <Group> may be ommited if repeated
// vect arrays not allowed (by ConfigModel)
//-------------------------------
// PID variables
CFGDEFA(vect, pidK,regCnt,  655.35,2,0.01, "proportional,integral,derivative [K]")
CFGDEFA(vect, pidL,regCnt,  100,1,1,       "limit [%]")
CFGDEFA(float,pidLo,regCnt, 100,1,1,       "output limit [%]")

CFGDEF(float, Ks_speed_low,     255,1,1,     "Speed: low speed value for multiplier [m/s]")
CFGDEF(float, Ks_speed_high,    255,1,1,     "high speed value for multiplier [m/s]")
CFGDEF(float, Ks_ailerons_low,  25.5,1,0.1,  "low speed ailerons multiplier [K]")
CFGDEF(float, Ks_ailerons_high, 2.55,1,0.01, "high speed ailerons multiplier [K]")
CFGDEF(float, Ks_elevator_low,  25.5,1,0.1,  "low speed elevator multiplier [K]")
CFGDEF(float, Ks_elevator_high, 2.55,1,0.01, "high speed elevator multiplier [K]")
CFGDEF(float, Ks_rudder_low,    25.5,1,0.1,  "low speed rudder multiplier [K]")
CFGDEF(float, Ks_rudder_high,   2.55,1,0.01, "high speed rudder multiplier [K]")
CFGDEF(float, Ks_steering_low,  25.5,1,0.1,  "low speed steering multiplier [K]")
CFGDEF(float, Ks_roll_high,     2.55,1,0.01, "high speed bank angle multiplier [K]")
CFGDEF(float, Ks_pitch_high,    2.55,1,0.01, "high speed pitch angle multiplier [K]")

CFGDEF(float, mix_elv_Kp, -1.27,1,0.01, "Mixer: Roll angle to Elevator mix")
CFGDEF(float, mix_elv_Lo, 100,1,1,      "Roll angle to Elevator limit")
CFGDEF(float, mix_rud_Kp, -12.7,1,0.1,  "Roll angle to Rudder mix")
CFGDEF(float, mix_rud_Lo, 100,1,1,      "Roll angle to Rudder limit (+/-) [%]")
CFGDEF(float, mix_thr_Kp, 25.5,1,0.1,   "Pitch angle to Throttle mix")
CFGDEF(float, mix_thr_Lo, 100,1,1,      "Pitch angle to Throttle limit (+/-) [%]")

CFGDEF(float, dyn_roll,  255,1,1, "Dynamics: roll angle change speed [deg/s]")
CFGDEF(float, dyn_pitch, 255,1,1, "pitch angle change speed [deg/s]")
CFGDEF(float, dyn_hystP, 25.5,1,0.1, "pitch error Kp hysterezis [deg]")

CFGDEF(vect,  imu_align, -180,2,0.1,      "AHRS: body frame align (roll,pitch,yaw) [-180..0..+180]")
CFGDEF(vect,  imu_gps_arm, -12.7,1,0.1,   "gps lever arm (x,y,z) [m]")
CFGDEF(float, imu_decl,   -90,2,0.01,     "Magnetic declination [deg]")
CFGDEF(float, imu_facc,   2.55,1,0.01,    "Accelerometers filter [0..1]")
CFGDEF(float, imu_fgyr,   2.55,1,0.01,    "Gyro filter [0..1]")

CFGDEF(float, wptSnap,   0,1,0,        "Waypoints: waypoint snap distance [m]")

CFGDEF(float, safety_dHome,    255000,1,1000,"Safety: max distance to go home [m]")
CFGDEF(float, safety_dHomeERS, 255000,1,1000,"suicide (ERS) distance [m]")
CFGDEF(float, safety_vspeedERS,50,1,1,       "max descending speed to launch ERS [m/s]")
CFGDEF(float, safety_Ve,       25.5,1,0.1,   "min autopilot battery voltage to go home [v]")
CFGDEF(float, safety_Vs,       45,1,0.1,     "min servo battery voltage to go home [v]")
CFGDEF(float, safety_VeERS,    25.5,1,0.1,   "min autopilot battery voltage to launch ERS [v]")
CFGDEF(float, safety_link,     255,1,1,      "data link timeout to go home [s]")

CFGDEF(float, rpm,     25500,1,100,   "Engine: cruise rpm [1/min]")
CFGDEF(float, rpm_idle,25500,1,100,   "idle rpm [1/min]")
CFGDEF(float, rpm_max, 25500,1,100,   "max rpm [1/min]")

CFGDEF(float, speed,           0,1,1,     "Flight: cruise speed [m/s]")
CFGDEF(float, speedMin,        0,1,1,     "min airspeed (stall) to pitch down [m/s]")
CFGDEF(float, speedMax,        0,1,1,     "max airspeed (VNE) to pitch up [m/s]")
CFGDEF(float, speedMinFlaps,   0,1,1,     "min airspeed (stall) with flaps down [m/s]")
CFGDEF(float, speedMaxFlaps,   0,1,1,     "max airspeed (VNE) with flaps down [m/s]")
CFGDEF(float, altitude,        2550,1,10,"safe altitude, HOME, TA mode [m]")
CFGDEF(float, stbyR,  -1270,1,10,         "standby radius, positive CW, negative CCW [m]")

CFGDEF(float, takeoff_flaps,     1,1,0.1,   "Takeoff: flaps level [0..1]")
CFGDEF(float, takeoff_thrRise,   1,1,0.01,  "initial throttle rise speed [1/s]")
CFGDEF(float, takeoff_liftSpeed, 100,1,1,   "airspeed to start liftoff [m/s]")
CFGDEF(float, takeoff_liftPitch, 100,1,1,   "liftoff pitch angle [deg]")
CFGDEF(float, takeoff_liftAlt,   100,1,1,   "altitude to start climbing [m]")
CFGDEF(float, takeoff_climbSpeed,100,1,1,   "climbing max airspeed [m/s]")
CFGDEF(float, takeoff_climbThr,  1,1,0.1,   "climbing throttle setting [0..1]")

CFGDEF(float, ld_appSpeed, 0,1,0,       "Landing: approach speed [m/s]")
CFGDEF(float, ld_flaps,    1,1,0.1,     "approach flaps level [0..1]")
CFGDEF(float, ld_finAlt,   255,1,0,     "approach final altitude AGL [m]")
CFGDEF(float, ld_finVSpeed,25.5,1,0.1,  "final descending speed [m/s]")
CFGDEF(float, ld_finPitchLo,25.5,1,0.1, "final descending pitch limit [deg/s]")
CFGDEF(float, ld_finPitchSpdKp,25.5,1,0.1, "final descending pitch speed Kp [K]")
CFGDEF(float, ld_tdPitch,  0,1,1,       "touchdown pitch bias [deg]")
CFGDEF(float, ld_tdAGL,    25.5,1,0.1,  "AGL altitude before touchdown [m]")
CFGDEF(float, ld_zeroAGL,  25.5,1,0.1,  "AGL altitude on the ground [m]")

CFGDEF(float, mekf_q_gyr, 25.5,1,0.1,   "MEKF: f(Gyro variance)")
CFGDEF(float, mekf_q_bgyr,25.5,1,0.1,   "f(Gyro bias variance)")
CFGDEF(float, mekf_q_pos, 25.5,1,0.1,   "f(pos variance)")
CFGDEF(float, mekf_q_vel, 25.5,1,0.1,   "f(Accelerometer variance)")
CFGDEF(float, mekf_q_bacc,25.5,1,0.1,   "f(Accelerometer bias variance)")
CFGDEF(float, mekf_r_mag, 25.5,1,0.1,   "Magnetometer variance")
CFGDEF(float, mekf_r_acc, 25.5,1,0.1,   "Accelerometer variance")
CFGDEF(float, mekf_r_pos, 25.5,1,0.1,   "GPS position variance")
CFGDEF(float, mekf_r_vel, 25.5,1,0.1,   "GPS velocity variance")

//=============================================================================
#undef CFGDEF
#undef CFGDEFA
