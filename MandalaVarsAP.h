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
#define AHRS_FREQ       100     // AHRS Update [Hz]
#define GPS_FREQ        5       // GPS Update [Hz]
#define CTR_FREQ        20      // RPTY (fast servo) update rate [Hz]
#define TELEMETRY_FREQ  10      // Telemetry send rate [Hz] MAX 10Hz!
#define SIM_FREQ        10      // Simulator servo send rate [Hz]
#define MAX_TELEMETRY   90      // max telemetry packet size [bytes]
//=============================================================================
#endif

//=============================================================================
//    Flight modes definitions. makes enum and mode_descr[], mode_names[]
// <mode enum name>, <description>
//------------------------------------------------------------------------------
#ifndef MODEDEF
#define MODEDEF(aname,adescr)
#endif
MODEDEF(EMG,     "Realtime control")
MODEDEF(RPV,     "Angles control")
MODEDEF(UAV,     "Heading control")
MODEDEF(WPT,     "Waypoints navigation")
MODEDEF(HOME,    "Go back home")
MODEDEF(TAKEOFF, "Takeoff")
MODEDEF(LANDING, "Landing")
MODEDEF(PLAND,   "Parachute Landing")
MODEDEF(STBY,    "Loiter around DNED")
//=============================================================================

//=============================================================================
//    PID Regulators definitions. makes enum and reg_descr[], reg_names[]
// <reg enum name>, <description>
//------------------------------------------------------------------------------
#ifndef REGDEF
#define REGDEF(aname,adescr)
#endif
REGDEF(Roll,     "Roll angle to Ailerons")
REGDEF(Pitch,    "Pitch angle to Elevator")
REGDEF(Thr,      "Airspeed to Throttle / RPM to Throttle")
REGDEF(Yaw,      "Yaw angle to Rudder")
REGDEF(RollH,    "Course to Desired Roll angle")
REGDEF(PitchH,   "Altitude to Desired Pitch angle")
REGDEF(Alt,      "VSpeed to Throttle/Altitude to Collective Pitch")
//=============================================================================


//=============================================================================
//--------- CONFIG --------------
#ifndef CFGDEF
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)
#endif
#ifndef CFGDEFA
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) CFGDEF(atype,aname,aspan,abytes,around,adescr)
#endif
//-------------------------------
// PID variables (the first 3 vars are hardcoded to GCU ConfigModel)
CFGDEFA(vect,       pidK,regCnt,   655.35,2,0.01, "PID coeffitients Kp,Ki,Kd")
CFGDEFA(vect,       pidL,regCnt,   100,1,1,       "PID limits Lp,Li,Ld [%]")
CFGDEFA(float,     pidLo,regCnt,  100,1,1,       "PID out limits Lo [%]")


// other variables (added to ConfigModel) automatically.
// description format:
// <Group>: text (vect axis names if any)
// vect axis names used to create text names in ConfigModel
// <Group> may be ommited if repeated
// vect arrays not allowed (by ConfigModel)
CFGDEF(float,  mix_elv_Kp,   2.55,1,0.01,  "Mixer: Roll angle to Elevator mix")
CFGDEF(float,  mix_elv_Lo,   100,1,1,      "Roll angle to Elevator limit")
CFGDEF(float,  mix_rud_Kp,   -12.7,1,0.1,  "Roll angle to Rudder mix")
CFGDEF(float,  mix_rud_Lo,   100,1,1,      "Roll angle to Rudder limit (+/-) [%]")
CFGDEF(float,  mix_thr_Kp,   25.5,1,0.1,   "Pitch angle to Throttle mix")
CFGDEF(float,  mix_thr_Lo,   100,1,1,      "Pitch angle to Throttle limit (+/-) [%]")

CFGDEF(float,   rollSpeed, 255,1,1,  "Dynamics: roll angle change speed [deg/s]")
CFGDEF(float,   pitchSpeed, 255,1,1,  "pitch angle change speed [deg/s]")

CFGDEF(vect,    imu_align,    -180,2,0.1,   "AHRS: body frame align (roll,pitch,yaw) [-180..0..+180]")
CFGDEF(vect,    theta_bias,   -180,2,0.1,   "AHRS: theta bias (roll,pitch,yaw) [-180..0..+180]")

CFGDEF(float,  wptSnap,      0,1,0,        "Waypoints: waypoint snap distance [m]")

CFGDEF(float,  safety_dHome,   255000,1,1000,  "Safety: turn home distance [m]")
CFGDEF(float,  safety_dHomeERS,   255000,1,1000,  "suicide (ERS) distance [m]")
CFGDEF(float,  safety_vspeedERS,   50,1,1,  "max descending speed to launch ERS [m/s]")

CFGDEF(float,  turnRate,   25.5,1,0.1,     "Sim: max turn rate [deg/s]")

CFGDEF(float,  hyst_dist,  25.5,1,0.1,     "Hysterezis: distance [m]")
CFGDEF(float,  hyst_spd,   25.5,1,0.1,     "speed hold [m/s]")
CFGDEF(float,  hyst_yaw,   25.5,1,0.1,     "heading hold [deg]")

CFGDEF(float,  eng_rpm,     25500,1,100,   "Engine: cruise rpm [1/min]")
CFGDEF(float,  eng_rpm_idle,25500,1,100,   "idle rpm [1/min]")
CFGDEF(float,  eng_rpm_max, 25500,1,100,   "max rpm [1/min]")


CFGDEF(float,  flight_speed,   0,1,0,      "Flight: cruise speed [m/s]")
CFGDEF(float,  flight_speedFlaps,0,1,0,    "airspeed limit with flaps down [m/s]")
CFGDEF(float,  flight_safeAlt, 2550,1,10,  "safe altitude, HOME, TA mode [m]")
CFGDEF(float,  flight_throttle, 1.0,1,0.01,  "cruise throttle setting [0..1]")

CFGDEF(float,  takeoff_Kp,      25.5,1,0.1,"Takeoff: climbing alt error coeffitient [deg]")
CFGDEF(float,  takeoff_Lp,      90,1,1,    "climbing pitch limit [deg]")
CFGDEF(float,  takeoff_thrSpeed, 1,1,0.1,  "throttle start speed [1/s]")
CFGDEF(float,  takeoff_liftSpeed,   100,1,1,   "airspeed to start liftoff [m/s]")
CFGDEF(float,  takeoff_liftPitch,   100,1,1,   "liftoff pitch angle [deg]")
CFGDEF(float,  takeoff_liftAlt,     100,1,1,   "altitude to start climbing [m]")

CFGDEF(float,  pland_pitch,   90,1,1,    "Parachute Landing: stall pitch [deg]")
CFGDEF(float,  pland_speed,   255,1,1,   "airspeed to open parachute [m/s]")

CFGDEF(float,  rw_dist,     2550,1,10,   "Runway Landing: approach distance [m]")
CFGDEF(float,  rw_turnDist, -1270,1,10,  "approach turn distance [m]")
CFGDEF(float,  rw_finAGL,   255,1,0,     "approach final altitude AGL [m]")
CFGDEF(float,  rw_finPitch, -12.7,1,0.1, "final pitch bias [deg]")
CFGDEF(float,  rw_finSpeed, 0,1,0,       "final speed [m/s]")
CFGDEF(float,  rw_tdPitch,  0,1,1,       "touchdown pitch bias [deg]")
CFGDEF(float,  rw_tdAGL,    12.7,1,0.1,    "AGL altitude before touchdown [m]")
CFGDEF(float,  rw_Kp,       2.55,1,0.01, "error multiplier [m]")
CFGDEF(float,  rw_Lp,       255,1,1,     "heading limit [deg]")

CFGDEF(float,  stby_R,  2550,1,10,     "Standby mode: radius [m]")
CFGDEF(float,  stby_Kp, 2.55,1,0.01,   "error multiplier [m]")
CFGDEF(float,  stby_Lp, 255,1,1,       "heading limit [deg]")

CFGDEF(float,  flaps_speed,     2.55,1,0.01, "Flaps: flaps speed [0..1]")
CFGDEF(float,  flaps_level,     2.55,1,0.01, "flaps level [0..1]")
CFGDEF(float,  flaps_levelTO,   2.55,1,0.01, "flaps level for TAKEOFF mode [0..1]")
//=============================================================================
#undef MODEDEF
#undef REGDEF
#undef CFGDEF
#undef CFGDEFA
