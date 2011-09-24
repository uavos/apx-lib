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
// Controls indexes
enum {jswRoll,jswPitch,jswThr,jswYaw,   jswCnt};
enum {ppmRoll,ppmPitch,ppmThr,ppmYaw,ppmSW,ppmFlaps,ppmBrake,   ppmCnt};
//-----------------------------------------------------------------------------
// Var Type enum
typedef enum {vt_void,vt_uint,vt_float,vt_vect,vt_sig}_var_type;
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
  VA_NUM_ARGS_IMPL(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1) )
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N
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
SIGDEF(service,   "Service packet down/up link <src-dadr>,<dadr-cmd>,<data..>")
SIGDEF(config,    "System configuration")
SIGDEF(downstream,"Downlink stream, all variables")
SIGDEF(debug,     "Debug <stdout> string")
SIGDEF(flightplan,"Flight plan data")
//------------------------------
// var packs
SIGDEF(imu, "IMU sensors data pack",\
       idx_acc, idx_gyro, idx_mag)
SIGDEF(gps, "GPS fix data pack",\
      idx_gps_lat, idx_gps_lon, idx_gps_hmsl, idx_gps_course, idx_gps_vNED, idx_gps_accuracy)
SIGDEF(ctr, "Fast controls",\
      idx_ctr_ailerons,idx_ctr_elevator,idx_ctr_throttle,idx_ctr_rudder,idx_ctr_wheel)
//------------------------------
// Internal use only
#define dl_reset_interval  10000    //reset snapshot interval [ms]
SIGDEF(autosend,  "Automatically forwarded variables to GCU",\
      idx_downstream, idx_debug, idx_service, idx_flightplan, idx_config )
//telemetry filter (don't send at all), calculated by mandala.extractTelemetry()
SIGDEF(dl_filter, "Downlink variables filter (calculated, not transmitted)",\
      idx_NED,idx_homeHDG,idx_dHome,idx_dWPT,idx_dNED,\
      idx_vXYZ,idx_dXYZ,idx_aXYZ,idx_crsRate,\
      idx_wpHDG,idx_rwDelta,\
      idx_wpcnt,idx_rwcnt)
//------------------------------
//=============================================================================
//    Mandala variables definitions
// type:           variable type: [uint, float, vect]
// name:           variable name, used for text also. access: var.name
// array:          for VARDEFA only (array size)
// span:           value span (for exchange compression), <0 = signed
// bytecount:      bytes number for data exchange compression
// description:    text description of the variable, used mostly in GCU
// if bitfield description contains slash '/' its status reported in console
//------------------------------------------------------------------------------
#ifndef VARDEF
#define VARDEF(atype,aname,aspan,abytes,adescr)
#endif
#ifndef VARDEFA
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) VARDEF(atype,aname,aspan,abytes,adescr)
#endif
#ifndef BITDEF
#define BITDEF(avarname,abitname,amask,adescr)
#endif
//--------- SPECIAL --------------
VARDEF(uint,    mode,      0,1,       "mode of operation: EMG,RPV,UAV...")
VARDEF(uint,    stage,     0,1,       "auto set to zero by mode change (maneuver stage)")
VARDEF(uint,    status,    0,1,       "status flags bitfield [status_xxx]")
BITDEF(status,    rc,      1,   "RC on/off")
BITDEF(status,    gps,     2,   "GPS available/lost")

//--------- IMU --------------
VARDEF(vect, theta, -180,2,  "roll,pitch,yaw [deg]")
VARDEF(vect, acc,   -100,2, "Ax,Ay,Az accelerations [m/s2]")
VARDEF(vect, gyro,  -300,2, "p,q,r angular rates [deg/s]")
VARDEF(vect, mag,   -1.25,1,   "Hx,Hy,Hz magnetic field vector [gauss]")

//--------- FLIGHT CONTROL --------------
VARDEF(float, airspeed,        655.35,2, "Airspeed [m/s]")
VARDEF(float, altitude,        6553.5,2, "Altitude [m]")
VARDEF(float, vspeed,          -100,2,   "Variometer [m/s]")

//--------- Measured by GPS --------------
VARDEF(float, gps_lat,     -180,4,     "latitude [deg]")
VARDEF(float, gps_lon,     -180,4,     "longitude [deg]")
VARDEF(float, gps_hmsl,    -10000,2,   "altitde above sea [m]")
VARDEF(vect,   gps_vNED,    -150,2,    "velocity north,east,down [m/s]")
VARDEF(float, gps_course,  -180,2,     "GPS course [deg]")
VARDEF(float, gps_home_lat, -180,4,    "home latitude [deg]")
VARDEF(float, gps_home_lon, -180,4,    "home longitude [deg]")
VARDEF(float, gps_home_hmsl,-10000,2,  "home altitde above sea [m]")
VARDEF(uint,  gps_time,      0,4,      "GPS UTC Time from 1970 1st Jan [sec]")
VARDEF(uint,  gps_SV,        0,1,      "GPS Satellites visible [number]")
VARDEF(uint,  gps_SU,        0,1,      "GPS Satellites used [number]")
VARDEF(vect,  gps_accuracy, 2.55,1,    "GPS accuracy estimation Hrz[m],Ver[m],Spd[m/s]")

//--------- BATTERY --------------
VARDEF(float, Ve,     25.5,1,   "autopilot battery voltage [v]")
VARDEF(float, Vs,     45,1,     "servo battery voltage [v]")
VARDEF(float, Vp,     0,1,      "payload battery voltage [v]")
VARDEF(float, Vg,     25.5,1,   "GCU battery voltage [v]")
VARDEF(uint,   power,  0,1,     "power status bitfield [on/off]")
BITDEF(power,   ap,      1,     "Avionics")
BITDEF(power,   servo,   2,     "Servo on/off")
BITDEF(power,   payload, 4,     "Payload activated/off")
BITDEF(power,   agl,     8,     "AGL sensor")
BITDEF(power,   ignition,16,    "Engine on/off")
BITDEF(power,   lights,  32,    "Lights on/off")
BITDEF(power,   taxi,    64,    "Taxi lights on/off")
BITDEF(power,   ice,    128,    "Anti-ice on/off")

//--------- OTHER AP SENSORS --------------
VARDEF(float,   rpm,    0,2,     "engine RPM [1/min]")
VARDEF(float,   agl,    25.5,1,  "Above Ground Level altitude [m]")

//--------- TEMPERATURES --------------
VARDEF(float, AT,    -100,1,    "ambient temperature [deg C]")
VARDEF(float, ET,     0,1,      "engine temperature [deg C]")
VARDEFA(float,  tsens,8,0,1,    "temperature sensors [C]")

//--------- OTHER SENSORS (information) --------------
VARDEF(float,   RSS,   1.0,1,   "Modem Receiver signal strength [0..1]")
VARDEF(float,   RSS_gcu,1.0,1,  "Modem GCU Receiver signal strength [0..1]")
VARDEF(float,   fuel,   1.0,1,  "Fuel [0..1]")

//--------- AUTOPILOT COMMAND --------------
VARDEF(vect,  cmd_theta,    -180,2,   "desired roll,pitch,yaw [deg]")
VARDEF(vect,  cmd_NED,      -10000,2, "desired north,east,down [m]")
VARDEF(float, cmd_course,   -180,2,   "desired course [deg]")
VARDEF(float, cmd_rpm,      25500,1,  "desired RPM [rpm]")
VARDEF(float, cmd_airspeed, 0,1,      "desired airspeed (for regThr) [m/s]")
VARDEF(float, cmd_vspeed,   -12.7,1,  "desired vertical speed (for regPitchH) [m/s]")

//--------- WAYPOINTS --------------
VARDEF(uint,  wpcnt,     0,1,       "number of waypoints [0...]")
VARDEF(uint,  wpType,    0,1,       "current waypoint type [wp_type]")
VARDEF(uint,  wpidx,     0,1,       "current waypoint [0...]")
VARDEF(uint,  rwcnt,     0,1,       "number of runways[0...]")

//--------- CONTROLS --------------
// fast controls
VARDEFA(float, jsw,jswCnt,    -1.0,1, "joystick command [-1..0..+1]")
VARDEFA(float, ppm,ppmCnt,    -1.0,1, "RC PPM command [-1..0..+1]")
VARDEF(float,  ctr_ailerons,  -1.0,1, "ailerons [-1..0..+1]")
VARDEF(float,  ctr_elevator,  -1.0,1, "elevator [-1..0..+1]")
VARDEF(float,  ctr_throttle,   1.0,1, "throttle [0..1]")
VARDEF(float,  ctr_rudder,    -1.0,1, "rudder [-1..0..+1]")
VARDEF(float,  ctr_wheel,     -1.0,1, "front wheel [-1..0..+1]")
// slow controls
VARDEF(float,  ctr_flaps,      1.0,1, "flaps [0..1]")
VARDEF(float,  ctr_airbrk,     1.0,1, "airbrakes [0..1]")
VARDEF(uint,   ctrb,           0,1,   "controls bitfield [on/off]")
BITDEF(ctrb,  gear,      1, "Landing gear retracted/extracted")
BITDEF(ctrb,  brake,     2, "Parking brake on/off")
BITDEF(ctrb,  ers,       4, "Parachute launched/off")

//--------- calculated by Mandala::calc() --------------
VARDEF(vect,  NED,     -10000,2, "north,east,down position [m]")
VARDEF(float,homeHDG, -180,2,   "heading to home position [deg]")
VARDEF(float,dHome,   0,2,      "distance to GCU [m]")
VARDEF(float,dWPT,    0,2,      "distance to waypoint [m]")
VARDEF(vect,  dNED,    -10000,2, "delta north,east,down [m]")
VARDEF(vect,  vXYZ,    -50,1,    "velocity bodyframe x,y,z [m/s]")
VARDEF(vect,  dXYZ,    -10000,2, "delta bodyframe x,y,z [m]")
VARDEF(vect,  aXYZ,    -100,1,   "accelerations by trajectory ax,ay,az [m/c2]")
VARDEF(float,crsRate, -50,1,    "trajectory course change rate [deg/s]")
VARDEF(float,wpHDG,   -180,2,   "current waypoint heading [deg]")
VARDEF(float,rwHDG,   -180,2,   "current runway heading [deg]")
VARDEF(float,rwDelta, -127,1,   "runway horizontal displacement [m]")

//=============================================================================
#undef SIGDEF
#undef VARDEF
#undef VARDEFA
#undef BITDEF
