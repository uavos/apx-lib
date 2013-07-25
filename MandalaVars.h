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
enum {vt_void=0,vt_uint,vt_float,vt_vect,vt_bits,vt_sig};
//=============================================================================
// Physical constants
#define EARTH_RATE   0.00007292115              // rotation rate of earth (rad/sec)
#define EARTH_RADIUS 6378137                    // earth semi-major axis radius (m)
#define ECCENTRICITY 0.0818191908426            // major eccentricity of earth ellipsoid
#define FLATTENING   0.0033528106650            // flattening of the ellipsoid
#define GRAVITY_0    9.7803730                  // zeroth coefficient for gravity model
#define GRAVITY_1    0.0052891                  // first coefficient for the gravity model
#define GRAVITY_2    0.0000059                  // second coefficient for the gravity model
#define GRAVITY_NOM  9.81                       // nominal gravity
#define SCHULER2     1.533421593170545E-06      // Sculer Frequency (rad/sec) Squared
#define R2D          57.29577951308232          // radians to degrees conversion factor
#define D2R          0.01745329251994           // degrees to radians conversion factor
#define FT2M         0.3048                     // feet to meters conversion factor
#define KTS2MS       0.5144                     // Knots to meters/sec conversion factor
#define PI           3.14159265358979           // pi
#define C_WGS84_a   6378137.0                   // earth equitorial radius
#define C_WGS84_b   6356752.3142                // earth polor radius
#define C_WGS84_f   ((C_WGS84_a-C_WGS84_b)/C_WGS84_a)
#define C_WIE       7.2321151467e-05            // WGS-84 earth rotation rate (rad/s)
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
//service packet, must be the first
SIGDEF(service,   "Service packet <node_sn>,<cmd>,<data..>")
//other protocols
SIGDEF(downstream,"Downlink stream <stream>")
SIGDEF(uplink,    "Uplink wrapped packet <var_idx>,<data..>")
SIGDEF(tagged,    "Tagged var <tag>,<var_idx>,<data..>")
SIGDEF(rawbus,    "RAW bus packet, forwarded to CAN <raw data..>")
SIGDEF(msg,       "Message string <text string>")
SIGDEF(ping,      "Ping packet, no reply expected")
SIGDEF(setb,      "Set bit (uplink only) <var_idx>,<mask>")
SIGDEF(clrb,      "Clear bit (uplink only) <var_idx>,<mask>")
SIGDEF(flightplan,"Flight plan data <packed fligthplan>")
SIGDEF(sim,       "Simulator wrapped packet <var_idx>,<data..>")
SIGDEF(tune,      "Tuning <parameter_id>,<data..>")
SIGDEF(data,      "Port data <port_id>,<data..>")
SIGDEF(uav_id,    "UAV identification data <packed UAV_ID>")
SIGDEF(formation, "Formation flight data <packed data>")
//------------------------------
// var packs
SIGDEF(imu, "IMU sensors data package",
       idx_acc, idx_gyro )
SIGDEF(gps, "GPS fix data package",
      idx_gps_lat, idx_gps_lon, idx_gps_hmsl, idx_gps_course, idx_gps_vNED)
SIGDEF(ctr, "Fast controls, sent to bus at fixed update rate",
      idx_ctr_ailerons,idx_ctr_elevator,idx_ctr_throttle,idx_ctr_rudder,idx_ctr_steering,idx_ctr_collective )
SIGDEF(pilot, "RC Pilot fast controls override", idx_rc_roll,idx_rc_pitch,idx_rc_throttle,idx_rc_yaw)
//------------------------------
// Auto update/send vars
SIGDEF(update,  "Auto send to bus when changed",
      idx_mode,idx_status,idx_error,idx_cmode,
      idx_power,idx_sw,
      idx_ctr_flaps,idx_ctr_airbrk,idx_ctr_brake,idx_ctr_mixture,idx_ctr_engine,idx_ctr_sweep,
      idx_ctrb,
      idx_cam_ch,idx_cam_ctr,idx_cam_opt,idx_cam_ctrb )

#define dl_reset_interval  10000    //reset snapshot interval [ms]
SIGDEF(autosend,  "Automatically forwarded variables to GCU",
       idx_service, idx_downstream, idx_uav_id, idx_msg, idx_ping, idx_flightplan, idx_rawbus, idx_data )

//telemetry filter (never send), also calculated by mandala.extractTelemetry()
SIGDEF(dl_filter, "Downlink variables filter (calculated, never transmitted over datalink)",
      idx_NED,idx_homeHDG,idx_dHome,idx_dWPT,idx_dN,idx_dE,idx_dAlt,
      idx_vXYZ,idx_dXYZ,
      idx_gSpeed,
      idx_wpHDG,idx_rwDelta,idx_rwDV,
      idx_wpcnt,idx_rwcnt,
      idx_rc_roll,idx_rc_pitch,idx_rc_throttle,idx_rc_yaw,
      idx_gcu_RSS,idx_gcu_Ve,idx_gcu_MT,idx_gcu_VSWR,
      idx_pstatic,
      idx_dl_period )
//------------------------------
#undef SIGDEF
//=============================================================================
//    Mandala variables definitions
// type:           variable type: [uint, float, vect]
// name:           variable name, used for text also. access: var.name
// array:          for VARDEFA only (array size)
// span:           value span (for 1 byte compression only), <0 = signed
// bytes:          bytes number for data exchange via bus (compression)
// tbytes:         bytes number for data exchange with GCU (more compression)
// description:    text description of the variable, used mostly in GCU
// if bitfield description contains slash '/' its status reported in console
//------------------------------------------------------------------------------
#ifndef VARDEF
#define VARDEF(atype,aname,aspan,abytes,atbytes,adescr)
#endif
#ifndef BITDEF
#define BITDEF(avarname,abitname,amask,adescr)
#endif
//--------- IMU --------------
VARDEF(vect,  theta, 0,2,2,     "attitude: roll,pitch,yaw [deg]")
VARDEF(vect,  acc,   -60,2,1,   "acceleration: Ax,Ay,Az [m/s2]")
VARDEF(vect,  gyro,  -700,2,1,  "angular rate: p,q,r [deg/s]")
VARDEF(vect,  mag,   -1.27,2,1, "magnetic field: Hx,Hy,Hz [gauss]")

//--------- FLIGHT CONTROL SENSORS --------------
VARDEF(float, airspeed,        0,2,2,   "Airspeed [m/s]")
VARDEF(float, altitude,        0,4,2,   "Altitude [m]")
VARDEF(float, vspeed,          0,2,2,   "Variometer [m/s]")

//--------- OTHER AP SENSORS --------------
VARDEF(float, rpm,    25500,2,1, "engine RPM [1/min]")
VARDEF(float, agl,    0,2,2,     "Above Ground Level altitude [m]")
VARDEF(float, slip,   -127,1,1,  "slip [deg]")
VARDEF(float, attack, -127,1,1,  "angle of attack [deg]")
VARDEF(float, venergy,-12.7,2,1, "compensated variometer [m/s]")

//--------- Measured by GPS --------------
VARDEF(float, gps_lat,     0,4,4,       "latitude [deg]")
VARDEF(float, gps_lon,     0,4,4,       "longitude [deg]")
VARDEF(float, gps_hmsl,    0,4,2,       "altitude above sea [m]")
VARDEF(vect,  gps_vNED,    0,2,2,       "velocity: Vnorth, Veast, Vdown [m/s]")
VARDEF(float, gps_course,  0,2,2,       "GPS course [deg]")
VARDEF(uint,  gps_time,    0,4,4,       "GPS UTC Time from 1970 1st Jan [sec]")

//--------- FAST CONTROLS --------------
VARDEF(float, ctr_ailerons,  -1.0,1,1, "ailerons [-1..0..+1]")
VARDEF(float, ctr_elevator,  -1.0,1,1, "elevator [-1..0..+1]")
VARDEF(float, ctr_throttle,   1.0,1,1, "throttle [0..1]")
VARDEF(float, ctr_rudder,    -1.0,1,1, "rudder [-1..0..+1]")

//--------- SLOW CONTROLS --------------
VARDEF(float, ctr_flaps,      1.0,1,1, "flaps [0..1]")
VARDEF(float, ctr_steering,  -1.0,1,1, "steering [-1..0..+1]")
VARDEF(float, ctr_collective,-1.0,1,1, "collective pitch [-1..0..+1]")
VARDEF(float, ctr_airbrk,     1.0,1,1, "airbrakes [0..1]")
VARDEF(float, ctr_brake,      1.0,1,1, "brake [0..1]")
VARDEF(float, ctr_mixture,    1.0,1,1, "mixture [0..1]")
VARDEF(float, ctr_engine,     1.0,1,1, "engine tuning [0..1]")
VARDEF(float, ctr_sweep,     -1.0,1,1, "sweep [-1..0..1]")

VARDEF(bits,  ctrb,  0,1,1,   "controls bitfield [on/off]")
BITDEF(ctrb,   ers,       1,  "ERS on/off")
BITDEF(ctrb,   rel,       2,  "Parachute released/locked")
BITDEF(ctrb,   rev,       4,  "Reverse activated/off")
BITDEF(ctrb,   gear,      8,  "Landing gear retracted/extracted")
BITDEF(ctrb,   drp,       16, "Drop-off open/locked")
BITDEF(ctrb,   pump,      32, "Fuel pump on/off")
BITDEF(ctrb,   xfeed,     64, "Crossfeed on/off")

//--------- AUTOPILOT COMMAND --------------
VARDEF(vect,  cmd_theta,    0,2,2,      "commanded attitude: roll,pitch,yaw [deg]")
VARDEF(float, cmd_N,        0,2,2,      "commanded north [m]")
VARDEF(float, cmd_E,        0,2,2,      "commanded east [m]")
VARDEF(float, cmd_course,   0,2,2,      "commanded course [deg]")
VARDEF(float, cmd_rpm,      25500,2,1,  "commanded RPM [rpm]")
VARDEF(float, cmd_altitude, 0,2,2,      "commanded altitude [m]")
VARDEF(float, cmd_airspeed, 0,2,1,      "commanded airspeed (for regThr) [m/s]")
VARDEF(float, cmd_vspeed,   -12.7,2,1,  "commanded vertical speed (for regPitchH) [m/s]")
VARDEF(float, cmd_slip,     -127,2,1,   "commanded slip [deg]")

//--------- Movement sensors --------------
VARDEF(vect,  radar_vXYZ,  0,2,2,       "radar velocity: Vx, Vy, Vz [m/s]")
VARDEF(vect,  radar_dXYZ,  0,2,2,       "radar delta: dx, dy, dz [m]")

//--------- OTHER SENSORS (information) --------------
VARDEF(float, fuel,  1.0,1,1,   "fuel capacity [0..1]")
VARDEF(float, frate, 0.01,1,1,  "fuel consumption rate [1/s]")
VARDEF(float, RSS,   1.0,1,1,   "modem signal strength [0..1]")

//--------- BATTERY --------------
VARDEF(float, Ve,     25.5,1,1,   "autopilot battery voltage [v]")
VARDEF(float, Vs,     25.5,1,1,   "servo battery voltage [v]")
VARDEF(float, Vp,     25.5,1,1,   "payload battery voltage [v]")
VARDEF(float, Vm,     45,1,1,     "engine battery voltage [v]")
VARDEF(float, Ie,     2.55,1,1,   "autopilot current [A]")
VARDEF(float, Is,     25.5,1,1,   "servo current [A]")
VARDEF(float, Ip,     25.5,1,1,   "payload current [A]")
VARDEF(float, Im,     0,1,1,      "engine current [A]")

//--------- TEMPERATURES --------------
VARDEF(float, AT,    -127,1,1,    "ambient temperature [C]")
VARDEF(float, RT,    -127,1,1,    "room temperature [C]")
VARDEF(float, MT,    -127,1,1,    "modem temperature [C]")
VARDEF(float, ET,     0,1,1,      "engine temperature [C]")
VARDEF(float, EGT,    900,1,1,    "exhaust gas temperature [C]")
VARDEF(float, OT,     0,1,1,      "oil temperature [C]")
VARDEF(float, OP,     25.5,1,1,   "oil pressure [atm]")

//--------- STATUS FLAGS --------------
VARDEF(uint,  stage,    0,1,1, "auto set to zero by mode change [maneuver stage]")
VARDEF(bits,  mode,     0,1,1, "flight mode")
BITDEF(mode,   EMG,     0,       "Realtime control")
BITDEF(mode,   RPV,     1,       "Angles control")
BITDEF(mode,   UAV,     2,       "Heading control")
BITDEF(mode,   WPT,     3,       "Waypoints navigation")
BITDEF(mode,   HOME,    4,       "Go back home")
BITDEF(mode,   TAKEOFF, 5,       "Takeoff")
BITDEF(mode,   LANDING, 6,       "Landing")
BITDEF(mode,   STBY,    7,       "Loiter around DNED")
BITDEF(mode,   DEMO,    8,       "Aerobatic demonstration")
BITDEF(mode,   RECOVER, 9,       "Recovery")

VARDEF(bits,  status,   0,1,1, "status flags bitfield [on/off]")
BITDEF(status, rc,      1,       "RC on/off")
BITDEF(status, gps,     2,       "GPS available/lost")
BITDEF(status, agl,     4,       "AGL available/off")
BITDEF(status, modem,   8,       "Uplink available/lost")
BITDEF(status, glide,   16,      "Glide status")

VARDEF(bits,  error,    0,1,1, "error flags bitfield [set/clear]")
BITDEF(error,  fatal,   1,       "Fatal error/ok")
BITDEF(error,  power,   2,       "Power supply error/ok")
BITDEF(error,  cas,     4,       "CAS error")
BITDEF(error,  pstatic, 8,       "Static pressure error/ok")
BITDEF(error,  gyro,    16,      "IMU gyros bias error/ok")
BITDEF(error,  engine,  32,      "Engine error/ok")
BITDEF(error,  rpm,     64,      "RPM sensor error/ok")
BITDEF(error,  ers,     128,     "ERS error/ok")

VARDEF(bits,  cmode,    0,1,1, "operation mode bitfield [on/off]")
BITDEF(cmode,  dlhd,    1,       "High precision downstream on/off")
BITDEF(cmode,  agl,     2,       "Altitude correction agl/off")
BITDEF(cmode,  ahrs,    4,       "AHRS mode inertial/gps")
BITDEF(cmode,  thrcut,  8,       "Throttle cut on/off")
BITDEF(cmode,  throvr,  16,      "Throttle override on/off")
BITDEF(cmode,  hover,   32,      "Stabilization mode hover/run")
BITDEF(cmode,  hdglook, 64,      "Heading control mode look/normal")

//--------- POWER CONTROL --------------
VARDEF(bits,  power,  0,1,1, "power controls bitfield [on/off]")
BITDEF(power,  ap,      1,     "Avionics")
BITDEF(power,  servo,   2,     "Servo on/off")
BITDEF(power,  ignition,4,     "Engine on/off")
BITDEF(power,  payload, 8,     "Payload activated/off")
BITDEF(power,  agl,     16,    "AGL sensor")
BITDEF(power,  satcom,  32,    "Satcom on/off")
BITDEF(power,  rfamp,   64,    "RF amplifier on/off")
BITDEF(power,  pcam,   128,    "Pilot camera on/off")

VARDEF(bits,  sw,     0,1,1, "operator controlled switches [on/off]")
BITDEF(sw,     lights,   1,    "Lights on/off")
BITDEF(sw,     taxi,     2,    "Taxi lights on/off")
BITDEF(sw,     ice,      4,    "Anti-ice on/off")
BITDEF(sw,     pheat,    8,    "Pitot tube heater on/off")
BITDEF(sw,     sw1,     16,    "switch 1 on/off")
BITDEF(sw,     sw2,     32,    "switch 2 on/off")
BITDEF(sw,     sw3,     64,    "switch 3 on/off")
BITDEF(sw,     sw4,    128,    "switch 4 on/off")

//--------- WAYPOINTS --------------
VARDEF(uint,  wpidx,     0,1,1,       "current waypoint [0...]")
VARDEF(uint,  wpType,    0,1,1,       "current waypoint type [wp_type]")
VARDEF(uint,  rwidx,     0,1,1,       "current runway [0...]")
VARDEF(float, rwHDG,     0,2,2,       "current runway heading [deg]")

//--------- dynamic tuning --------------
VARDEF(float, windSpd,      25.5,2,1,   "wind speed [m/s]")
VARDEF(float, windHdg,      360,2,1,    "wind direction to 0..360 [deg]")
VARDEF(uint,  errcode,      0,1,1,      "error code")
VARDEF(float, performance,  0,2,2,      "Aircraft performance [K]")
VARDEF(float, energy,       0,2,2,      "Static energy [K]")
VARDEF(float, corr,         0,2,2,      "Correlator output [K]")
VARDEF(float, corrTAS,      25.5,2,1,   "CAS to TAS multiplier [K]")
VARDEF(float, rwAdj,        -127,1,1,   "runway displacement adjust during takeoff or landing [m]")
VARDEF(float, gps_home_lat, 0,4,4,      "home latitude [deg]")
VARDEF(float, gps_home_lon, 0,4,4,      "home longitude [deg]")
VARDEF(float, gps_home_hmsl,0,4,4,      "home altitde above sea [m]")
VARDEF(uint,  gps_SV,       0,1,1,      "GPS Satellites visible [number]")
VARDEF(uint,  gps_SU,       0,1,1,      "GPS Satellites used [number]")
VARDEF(float, pstatic_gnd,  0,4,4,      "Static pressure on ground level [inHg]")
VARDEF(float, safe_altitude,0,4,4,      "Safe altitude [m]")
VARDEF(float, cruise_airspeed,0,2,2,    "Cruise airspeed [m/s]")

//--------- user values --------------
VARDEF(float, user1,      0,4,2,   "User value 1")
VARDEF(float, user2,      0,4,2,   "User value 2")
VARDEF(float, user3,      0,4,2,   "User value 3")
VARDEF(float, user4,      0,4,2,   "User value 4")

//--------- CAM CONTROL --------------
VARDEF(uint, cam_ch,    0,1,1,          "video channel")
VARDEF(bits, cam_ctr,   0,1,1,          "camera control type")
BITDEF(cam_ctr, camoff,  0,               "Camera off")
BITDEF(cam_ctr, stab,    1,               "gyro stabilization")
BITDEF(cam_ctr, hold,    2,               "attitude hold")
BITDEF(cam_ctr, pos,     3,               "position tracking")
BITDEF(cam_ctr, fixed,   4,               "fixed position")
VARDEF(float, cam_heading, 0,4,2,       "camera heading [deg]")
VARDEF(float, cam_pitch,   0,4,2,       "camera pitch [deg]")
VARDEF(float, cam_zoom,    1.0,1,1,     "camera zoom level [0..1]")
VARDEF(float, cam_focus,   1.0,1,1,     "camera focus [0..1]")
VARDEF(float, cam_bias_h,  0,4,2,       "camera heading stability bias [deg/s]")
VARDEF(float, cam_bias_p,  0,4,2,       "camera pitch stability bias [deg/s]")
VARDEF(bits, cam_opt,  0,1,1,           "camera options [on/off]")
BITDEF(cam_opt, PF,      1,               "Picture flip on/off")
BITDEF(cam_opt, NIR,     2,               "NIR filter on/off")
BITDEF(cam_opt, DSP,     4,               "Display information on/off")
BITDEF(cam_opt, FM,      8,               "Focus mode infinity/auto")
VARDEF(float, cam_lat, 0,4,4,           "camera track latitude [deg]")
VARDEF(float, cam_lon, 0,4,4,           "camera track longitude [deg]")
VARDEF(float, cam_alt, 0,4,2,           "camera track altitude [m]")
VARDEF(bits, cam_ctrb, 0,1,1,           "camera controls bitfield [on/off]")
BITDEF(cam_ctrb, shot,  1,                "Snapshot")
BITDEF(cam_ctrb, ashot, 2,                "Series snapshots")

//--------- calculated by Mandala::calc() --------------
VARDEF(vect,  NED,     0,4,2,           "local position: north,east,down [m]")
VARDEF(float, homeHDG, 0,4,2,           "heading to home position [deg]")
VARDEF(float, dHome,   0,4,2,           "distance to GCU [m]")
VARDEF(float, dWPT,    0,4,2,           "distance to waypoint [m]")
VARDEF(float, dN,      0,4,2,           "delta north [m]")
VARDEF(float, dE,      0,4,2,           "delta east [m]")
VARDEF(float, dAlt,    0,4,2,           "delta altitude [m]")
VARDEF(vect,  vXYZ,    0,4,2,           "bodyframe velocity: Vx,Vy,Vz [m/s]")
VARDEF(vect,  dXYZ,    0,4,2,           "bodyframe delta: dx,dy,dz [m]")
VARDEF(float, gSpeed,  0,4,2,           "ground speed [m]")
VARDEF(float, wpHDG,   0,4,2,           "current waypoint heading [deg]")
VARDEF(float, rwDelta, 0,4,2,           "runway alignment [m]")
VARDEF(float, rwDV,    0,4,2,           "runway alignment velocity [m/s]")

//--------- filtered dynamic tuning --------------
VARDEF(uint,  dl_period, 0,2,2,         "downlink period [ms]")
VARDEF(uint,  wpcnt,     0,1,1,         "number of waypoints [0...]")
VARDEF(uint,  rwcnt,     0,1,1,         "number of runways [0...]")
VARDEF(float, pstatic,   0,4,2,         "Static pressure [inHg]")

//--------- PILOT CONTROLS --------------
VARDEF(float, rc_roll,    -1.0,1,1, "RC roll [-1..0..+1]")
VARDEF(float, rc_pitch,   -1.0,1,1, "RC pitch [-1..0..+1]")
VARDEF(float, rc_throttle, 1.0,1,1, "RC throttle [0..1]")
VARDEF(float, rc_yaw,     -1.0,1,1, "RC yaw [-1..0..+1]")

//--------- GCU use only --------------
VARDEF(float, gcu_RSS,   1.0,1,1, "GCU modem signal strength [0..1]")
VARDEF(float, gcu_Ve,   25.5,1,1, "GCU battery voltage [v]")
VARDEF(float, gcu_MT,   -127,1,1, "GCU modem temperature [C]")
VARDEF(float, gcu_VSWR, 25.5,1,1, "GCU modem standing wave ratio [VSWR]")
//=============================================================================
#undef VARDEF
#undef BITDEF
