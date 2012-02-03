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
#ifndef MANDALA_CONSTANTS
#define MANDALA_CONSTANTS
//=============================================================================
// Mandala constants
//=============================================================================
#define idxPAD  64      //start index for regular vars
//-----------------------------------------------------------------------------
// Var Type enum
enum {vt_void,vt_uint,vt_float,vt_vect,vt_sig,vt_core_top};
//=============================================================================
// Physical constants
#define EARTH_RATE   0.00007292115   // rotation rate of earth (rad/sec)
#define EARTH_RADIUS 6378137         // earth semi-major axis radius (m)
#define ECCENTRICITY 0.0818191908426 // major eccentricity of earth ellipsoid
#define FLATTENING   0.0033528106650 // flattening of the ellipsoid
#define GRAVITY_0    9.7803730       // zeroth coefficient for gravity model
#define GRAVITY_1    0.0052891       // first coefficient for the gravity model
#define GRAVITY_2    0.0000059       // second coefficient for the gravity model
#define GRAVITY_NOM  9.81            // nominal gravity
#define SCHULER2     1.533421593170545E-06 // Sculer Frequency (rad/sec) Squared
#define R2D          57.29577951308232     // radians to degrees conversion factor
#define D2R          0.01745329251994      // degrees to radians conversion factor
#define FT2M         0.3048                // feet to meters conversion factor
#define KTS2ms       0.5144                // Knots to meters/sec conversion factor
#define PI           3.14159265358979      // pi
#define MAG_DEC     -0.270944862           //magnetic declination of Stanford (rad): -15.15
#define MM2M         0.001                 //mm to m

#define C_WGS84_a   6378137.0         // WGS-84 semimajor axis (m)
#define C_WGS84_a_F 6378137.0f
#define C_WGS84_b   6356752.3142      // WGS-84 semiminor axis (m)
#define C_WGS84_b_F 6356752.3142f
#define C_WIE       7.2321151467e-05  // WGS-84 earth rotation rate (rad/s)
#define C_WIE_F     7.2321151467e-05f
//=============================================================================
#define VA_NUM_ARGS(...) ( sizeof(#__VA_ARGS__)==sizeof("")?0:\
  VA_NUM_ARGS_IMPL(__VA_ARGS__, 32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1) )
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,N,...) N
//=============================================================================
#endif
//=============================================================================
//=============================================================================
//=============================================================================
// special variables - signatures
#ifndef SIGDEF
#define SIGDEF(aname,adescr,...)
#endif
//------------------------------
// special protocols
SIGDEF(config,    "System configuration")
SIGDEF(downstream,"Downlink stream, all variables")
SIGDEF(debug,     "Debug <stdout> string")
SIGDEF(flightplan,"Flight plan data")
SIGDEF(health,    "Health status <nodes cnt>,<node_adr><node_health>...")
SIGDEF(service,   "Service packet down/up link <src-dadr>,<dadr-cmd>,<data..>")
SIGDEF(setb,      "Set bit <var_idx>,<data>")
SIGDEF(clrb,       "Clear bit <var_idx>,<data>")
//------------------------------
// var packs
SIGDEF(imu, "IMU sensors data pack",
       idx_acc, idx_gyro )
SIGDEF(gps, "GPS fix data pack",
      idx_gps_lat, idx_gps_lon, idx_gps_hmsl, idx_gps_course, idx_gps_vNED)
SIGDEF(ctr, "Fast controls",
      idx_ctr_ailerons,idx_ctr_elevator,idx_ctr_throttle,idx_ctr_rudder,idx_ctr_wheel )
SIGDEF(pilot, "RC Pilot fast controls override", idx_rc_roll,idx_rc_pitch,idx_rc_throttle,idx_rc_yaw,idx_rc_wheel)
SIGDEF(ctr_gcu, "GCU Pilot fast controls", idx_rc_roll,idx_rc_pitch,idx_rc_throttle,idx_rc_yaw,idx_rc_wheel)
//------------------------------
// Internal use only
SIGDEF(update,  "Auto send to bus when changed",
      idx_mode,idx_status,idx_power,
      idx_ctrb,idx_ctr_airbrk,idx_ctr_flaps,
      idx_cam_ctr,idx_cam_heading,idx_cam_pitch )

#define dl_reset_interval  10000    //reset snapshot interval [ms]
SIGDEF(autosend,  "Automatically forwarded variables to GCU",
      idx_downstream, idx_debug, idx_flightplan, idx_config, idx_service, idx_health )

//telemetry filter (don't send at all), calculated by mandala.extractTelemetry()
SIGDEF(dl_filter, "Downlink variables filter (calculated, not transmitted)",
      idx_homeHDG,idx_dHome,idx_dWPT,idx_dN,idx_dE,idx_dAlt,
      idx_dXYZ,
      idx_wpHDG,idx_rwDelta,
      idx_wpcnt,idx_rwcnt,
      idx_NED,idx_vXYZ,idx_aXYZ,idx_crsRate,
      idx_rc_roll,idx_rc_pitch,idx_rc_throttle,idx_rc_yaw,idx_rc_wheel,
      idx_gcu_RSS, idx_gcu_Ve, idx_gcu_MT,
      idx_pstatic )
//------------------------------
#undef SIGDEF
//=============================================================================
//    Mandala variables definitions
// type:           variable type: [uint, float, vect]
// name:           variable name, used for text also. access: var.name
// array:          for VARDEFA only (array size)
// span:           value span (for exchange compression), <0 = signed
// bytes:          bytes number for data exchange compression
// tbytes:         bytes number for data exchange with GCU
// description:    text description of the variable, used mostly in GCU
// if bitfield description contains slash '/' its status reported in console
//------------------------------------------------------------------------------
#ifndef VARDEF
#define VARDEF(atype,aname,aspan,abytes,atbytes,adescr)
#endif
#ifndef BITDEF
#define BITDEF(avarname,abitname,amask,adescr)
#endif
//--------- SPECIAL --------------
VARDEF(uint,  mode,     0,1,0,  "flight mode")
BITDEF(mode,   EMG,     0,       "Realtime control")
BITDEF(mode,   RPV,     1,       "Angles control")
BITDEF(mode,   UAV,     2,       "Heading control")
BITDEF(mode,   WPT,     3,       "Waypoints navigation")
BITDEF(mode,   HOME,    4,       "Go back home")
BITDEF(mode,   TAKEOFF, 5,       "Takeoff")
BITDEF(mode,   LANDING, 6,       "Landing")
BITDEF(mode,   STBY,    7,       "Loiter around DNED")
BITDEF(mode,   DEMO,    8,       "Aerobatic demonstration")
VARDEF(uint,  stage,    0,1,0,  "auto set to zero by mode change [maneuver stage]")
VARDEF(uint,  status,   0,1,0,  "status flags bitfield")
BITDEF(status, rc,      1,       "RC on/off")
BITDEF(status, gps,     2,       "GPS available/lost")

//--------- IMU --------------
VARDEF(vect,  theta, -180,2,0,   "attitude: roll,pitch,yaw [deg]")
VARDEF(vect,  acc,   -60,2,1,    "acceleration: Ax,Ay,Az [m/s2]")
VARDEF(vect,  gyro,  -700,2,1,   "angular rate: p,q,r [deg/s]")
VARDEF(vect,  mag,   -1.27,2,1,  "magnetic field: Hx,Hy,Hz [gauss]")

//--------- FLIGHT CONTROL --------------
VARDEF(float, airspeed,        655.35,2,0, "Airspeed [m/s]")
VARDEF(float, altitude,        15000.0,4,2, "Altitude [m]")
VARDEF(float, vspeed,          -100,2,0,   "Variometer [m/s]")
VARDEF(float, pstatic,        35.0,4,2,    "Static pressure [inHg]")
VARDEF(float, pstatic_gnd,    35.0,4,0,    "Static pressure on ground level [inHg]")

//--------- Measured by GPS --------------
VARDEF(float, gps_lat,     -180,4,0,     "latitude [deg]")
VARDEF(float, gps_lon,     -180,4,0,     "longitude [deg]")
VARDEF(float, gps_hmsl,    -10000,2,0,   "altitude above sea [m]")
VARDEF(vect,  gps_vNED,    -150,2,1,     "velocity: Vnorth, Veast, Vdown [m/s]")
VARDEF(float, gps_course,  -180,2,1,     "GPS course [deg]")
VARDEF(float, gps_home_lat, -180,4,0,    "home latitude [deg]")
VARDEF(float, gps_home_lon, -180,4,0,    "home longitude [deg]")
VARDEF(float, gps_home_hmsl,-10000,2,0,  "home altitde above sea [m]")
VARDEF(uint,  gps_time,      0,4,0,      "GPS UTC Time from 1970 1st Jan [sec]")
VARDEF(uint,  gps_SV,        0,1,0,      "GPS Satellites visible [number]")
VARDEF(uint,  gps_SU,        0,1,0,      "GPS Satellites used [number]")
VARDEF(vect,  gps_accuracy, 2.55,1,0,    "GPS accuracy: hrz, ver, spd [m, m/s]")

//--------- BATTERY --------------
VARDEF(float, Ve,     25.5,1,0,   "autopilot battery voltage [v]")
VARDEF(float, Vs,     25.5,1,0,   "servo battery voltage [v]")
VARDEF(float, Vp,     25.5,1,0,   "payload battery voltage [v]")
VARDEF(float, Vm,     45,1,0,     "engine battery voltage [v]")
VARDEF(float, Ie,     2.55,1,0,   "autopilot current [A]")
VARDEF(float, Is,     25.5,1,0,   "servo current [A]")
VARDEF(float, Ip,     25.5,1,0,   "payload current [A]")
VARDEF(float, Im,     255,1,0,    "engine current [A]")

//--------- OTHER AP SENSORS --------------
VARDEF(float, rpm,    0,2,0,     "engine RPM [1/min]")
VARDEF(float, agl,    25.5,1,0,  "Above Ground Level altitude [m]")

//--------- TEMPERATURES --------------
VARDEF(float, AT,    -127,1,0,    "ambient temperature [C]")
VARDEF(float, RT,    -127,1,0,    "room temperature [C]")
VARDEF(float, MT,    -127,1,0,    "modem temperature [C]")
VARDEF(float, ET,     0,1,0,      "engine temperature [C]")
VARDEF(float, EGT,    900,1,0,    "exhaust gas temperature [C]")

//--------- OTHER SENSORS (information) --------------
VARDEF(float, RSS,   1.0,1,0,   "modem signal strength [0..1]")
VARDEF(float, fuel,   1.0,1,0,  "fuel capacity [0..1]")

//--------- AUTOPILOT COMMAND --------------
VARDEF(vect,  cmd_theta,    -180,2,0,   "desired attitude: roll,pitch,yaw [deg]")
VARDEF(float, cmd_N,        -10000,2,0, "desired north [m]")
VARDEF(float, cmd_E,        -10000,2,0, "desired east [m]")
VARDEF(float, cmd_course,   -180,2,0,   "desired course [deg]")
VARDEF(float, cmd_rpm,      25500,1,0,  "desired RPM [rpm]")
VARDEF(float, cmd_altitude, -10000,2,0, "desired altitude [m]")
VARDEF(float, cmd_airspeed, 0,1,0,      "desired airspeed (for regThr) [m/s]")
VARDEF(float, cmd_vspeed,   -12.7,1,0,  "desired vertical speed (for regPitchH) [m/s]")

//--------- WAYPOINTS --------------
VARDEF(uint,  wpcnt,     0,1,0,       "number of waypoints [0...]")
VARDEF(uint,  wpType,    0,1,0,       "current waypoint type [wp_type]")
VARDEF(uint,  wpidx,     0,1,0,       "current waypoint [0...]")
VARDEF(uint,  rwcnt,     0,1,0,       "number of runways[0...]")

//--------- CONTROLS --------------
// fast controls
VARDEF(float, ctr_ailerons,  -1.0,1,0, "ailerons [-1..0..+1]")
VARDEF(float, ctr_elevator,  -1.0,1,0, "elevator [-1..0..+1]")
VARDEF(float, ctr_throttle,   1.0,1,0, "throttle [0..1]")
VARDEF(float, ctr_rudder,    -1.0,1,0, "rudder [-1..0..+1]")
VARDEF(float, ctr_wheel,     -1.0,1,0, "front wheel [-1..0..+1]")
// slow controls
VARDEF(float, ctr_flaps,      1.0,1,0, "flaps [0..1]")
VARDEF(float, ctr_airbrk,     1.0,1,0, "airbrakes [0..1]")
VARDEF(uint,  ctrb,           0,1,0,   "controls bitfield [on/off]")
BITDEF(ctrb,   gear,      1, "Landing gear retracted/extracted")
BITDEF(ctrb,   brake,     2, "Parking brake on/off")
BITDEF(ctrb,   ers,       4, "ERS on/off")

//--------- CAM CONTROL --------------
VARDEF(float, cam_heading, -180.0,2,0, "camera heading [deg]")
VARDEF(float, cam_pitch,   -180.0,2,0, "camera pitch [deg]")
VARDEF(float, cam_lat,     -180,4,0,   "cam track latitude [deg]")
VARDEF(float, cam_lon,     -180,4,0,   "cam track longitude [deg]")
VARDEF(float, cam_alt,     -10000,2,0, "cam track altitude [m]")
VARDEF(uint, cam_ctr,  0,1,0, "camera control type")
BITDEF(cam_ctr, camoff,  0,  "Camera off")
BITDEF(cam_ctr, atrack,  1,  "Camera attitude hold")
BITDEF(cam_ctr, ptrack,  2,  "Camera position tracking")
BITDEF(cam_ctr, front,   3,  "Front facing camera")

//--------- POWER CONTROL --------------
VARDEF(uint,  power,  0,1,0,   "power status bitfield [on/off]")
BITDEF(power,  ap,      1,     "Avionics")
BITDEF(power,  servo,   2,     "Servo on/off")
BITDEF(power,  payload, 4,     "Payload activated/off")
BITDEF(power,  agl,     8,     "AGL sensor")
BITDEF(power,  ignition,16,    "Engine on/off")
BITDEF(power,  lights,  32,    "Lights on/off")
BITDEF(power,  taxi,    64,    "Taxi lights on/off")
BITDEF(power,  ice,    128,    "Anti-ice on/off")

//--------- PILOT CONTROLS --------------
VARDEF(float, rc_roll,    -1.0,1,0, "RC Roll [-1..0..+1]")
VARDEF(float, rc_pitch,   -1.0,1,0, "RC Pitch [-1..0..+1]")
VARDEF(float, rc_throttle, 1.0,1,0, "RC Throttle [0..1]")
VARDEF(float, rc_yaw,     -1.0,1,0, "RC Yaw [-1..0..+1]")
VARDEF(float, rc_wheel,   -1.0,1,0, "RC Front Wheel [-1..0..+1]")

//--------- calculated by Mandala::calc() --------------
VARDEF(vect,  NED,     -10000,2,0, "local position: north,east,down [m]")
VARDEF(float, homeHDG, -180,2,0,   "heading to home position [deg]")
VARDEF(float, dHome,   0,2,0,      "distance to GCU [m]")
VARDEF(float, dWPT,    0,2,0,      "distance to waypoint [m]")
VARDEF(float, dN,      -10000,2,0, "delta north [m]")
VARDEF(float, dE,      -10000,2,0, "delta east [m]")
VARDEF(float, dAlt,    -10000,2,0, "delta altitude [m]")
VARDEF(vect,  vXYZ,    -50,1,0,    "bodyframe velocity: Vx,Vy,Vz [m/s]")
VARDEF(vect,  dXYZ,    -10000,2,0, "bodyframe delta: dx,dy,dz [m]")
VARDEF(vect,  aXYZ,    -100,2,0,   "acceleration by trajectory: atx,aty,atz [m/s2]")
VARDEF(float, crsRate, -50,1,0,    "trajectory course change rate [deg/s]")
VARDEF(float, wpHDG,   -180,2,0,   "current waypoint heading [deg]")
VARDEF(float, rwHDG,   -180,2,0,   "current runway heading [deg]")
VARDEF(float, rwDelta, -127,1,0,   "runway horizontal displacement [m]")

//--------- GCU use only --------------
VARDEF(float, gcu_RSS,  1.0,1,0,  "GCU modem signal strength [0..1]")
VARDEF(float, gcu_Ve,   25.5,1,0, "GCU battery voltage [v]")
VARDEF(float, gcu_MT,   -127,1,0, "GCU modem temperature [C]")
//=============================================================================
#undef VARDEF
#undef BITDEF
