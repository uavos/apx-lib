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
#define maxVars 256
#define idxCFG  200     //configuration variables
#define idxSIG  160     //special variables - signatures
#define idxCMD  200     //special variables - commands
//-----------------------------------------------------------------------------
#define AHRS_FREQ       100     // AHRS Update [Hz]
#define GPS_FREQ        5       // GPS Update [Hz]
#define CTR_FREQ        20      // RPTY (fast servo) update rate [Hz]
#define TELEMETRY_FREQ  10      // Telemetry send rate [Hz] MAX 10Hz!
#define SIM_FREQ        10      // Simulator servo send rate [Hz]
#define MAX_TELEMETRY   90      // max telemetry packet size [bytes]
//-----------------------------------------------------------------------------
// Controls indexes
enum {jRoll,jPitch,jThr,jYaw,jFlaps};
typedef enum {vt_void,vt_uint,vt_double,vt_Vect,vt_sig}_var_type;
//-----------------------------------------------------------------------------
//#define   srvCnt    10    // servos cnt
#define   jswCnt    4     // joystick axes
#define   ppmCnt    5     // RC PPM channels cnt
//-----------------------------------------------------------------------------
//=============================================================================
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
//-----------------------------------------------------------------------------
// special variables - signatures
#ifndef SIGDEF
#define SIGDEF(aname,adescr,...)
#endif
SIGDEF(imu, "IMU sensors data pack",\
       idx_acc, idx_gyro, idx_mag)
SIGDEF(gps, "GPS fix data pack",\
      idx_gps_lat, idx_gps_lon, idx_gps_hmsl, idx_gps_course, idx_gps_vNED, idx_gps_accuracy)
SIGDEF(ctr, "Fast controls",\
      idx_ctr_ailerons,idx_ctr_elevator,idx_ctr_throttle,idx_ctr_rudder,idx_ctr_wheel)
//telemetry filter (send with skip factor X), if changed fast but no need for update
#define dl_slow_factor  10
SIGDEF(dl_slow, "Downlink variables filter (slow update rate)",\
      idx_Ve,idx_Vs,idx_Vp,idx_AT,idx_ET,idx_fuel,idx_tsens)
#define dl_reset_interval  10000    //reset snapshot interval [ms]
//telemetry filter (don't send at all), calculated by mandala.extractTelemetry()
SIGDEF(dl_filter, "Downlink variables filter (calculated, not transmitted)",\
      idx_NED,idx_homeHDG,idx_dHome,idx_dWPT,idx_dNED,\
      idx_vXYZ,idx_dXYZ,idx_aXYZ,idx_crsRate,\
      idx_wpHDG,idx_rwDelta,\
      idx_wpcnt,idx_rwcnt)
// telemetry variables
SIGDEF(downstream,"Downlink stream, all variables")
SIGDEF(debug,     "Debug <stdout> string forward to GCU")
SIGDEF(service,   "Servive packet down/up link <src-dadr>,<dadr-cmd>,<data..>")
SIGDEF(flightplan,"Flight plan packed data")
//SIGDEF(uplink)    // special command uplink
// static signatures
SIGDEF(config,    "List of all configuration vars (>=idxCFG)")
//modem special
SIGDEF(autosend,  "Automatically forwarded variables to GCU",\
      idx_downstream, idx_debug, idx_service, idx_flightplan, idx_config )
//=============================================================================
//    Mandala variables definitions
// type:           variable type: [uint, double, Vect]
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
VARDEF(Vect, theta, -180,2,  "roll,pitch,yaw [deg]")
VARDEF(Vect, acc,   -100,2, "Ax,Ay,Az accelerations [m/s2]")
VARDEF(Vect, gyro,  -300,2, "p,q,r angular rates [deg/s]")
VARDEF(Vect, mag,   -1.25,1,   "Hx,Hy,Hz magnetic field vector [gauss]")

//--------- Measured by GPS --------------
VARDEF(double, gps_lat,     -180,4,     "latitude [deg]")
VARDEF(double, gps_lon,     -180,4,     "longitude [deg]")
VARDEF(double, gps_hmsl,    -10000,2,   "altitde above sea [m]")
VARDEF(Vect,   gps_vNED,    -150,2,     "velocity north,east,down [m/s]")
VARDEF(double, gps_course,  -180,2,     "GPS course [deg]")
VARDEF(Vect,   gps_accuracy, 2.55,1,    "GPS accuracy estimation Hrz[m],Ver[m],Spd[m/s]")
VARDEF(double, gps_home_lat, -180,4,    "home latitude [deg]")
VARDEF(double, gps_home_lon, -180,4,    "home longitude [deg]")
VARDEF(double, gps_home_hmsl,-10000,2,  "home altitde above sea [m]")

//--------- PRESSURE --------------
VARDEF(double, airspeed,        100,1,    "barometric airspeed [m/s]")
VARDEF(double, pstatic,         6553.5,2,  "barometric pressure AGL [m]")

//--------- BATTERY --------------
VARDEF(double, Ve,     25.5,2,     "autopilot battery voltage [v]")
VARDEF(double, Vs,     45,2,     "servo battery voltage [v]")
VARDEF(double, Vp,     0,2,          "payload battery voltage [v]")
VARDEF(uint,   power,  0,1,          "power status bitfield [on/off]")
BITDEF(power,   ap,      1,   "Avionics")
BITDEF(power,   servo,   2,   "Servo on/off")
BITDEF(power,   payload, 4,   "Payload activated/off")
BITDEF(power,   agl,     8,   "AGL sensor")
BITDEF(power,   ignition,16,  "Engine on/off")
BITDEF(power,   lights,  32,  "Lights on/off")
BITDEF(power,   taxi,    64,  "Taxi lights on/off")

//--------- TEMPERATURES --------------
VARDEF(double, AT,    -100,1,      "ambient temperature [deg C]")
VARDEF(double, ET,     0,1,        "engine temperature [deg C]")

//--------- CONTROLS --------------
// fast controls
VARDEFA(double, jsw,jswCnt,    -1.0,1, "joystick command [-1..0..+1]")
VARDEFA(double, ppm,ppmCnt,    -1.0,1, "RC PPM command [-1..0..+1]")
VARDEF(double,  ctr_ailerons,  -1.0,1, "ailerons [-1..0..+1]")
VARDEF(double,  ctr_elevator,  -1.0,1, "elevator [-1..0..+1]")
VARDEF(double,  ctr_throttle,   1.0,1, "throttle [0..1]")
VARDEF(double,  ctr_rudder,    -1.0,1, "rudder [-1..0..+1]")
VARDEF(double,  ctr_wheel,     -1.0,1, "front wheel [-1..0..+1]")
// slow controls
VARDEF(double,  ctr_flaps,      1.0,1, "flaps [0..1]")
VARDEF(double,  ctr_airbrk,     1.0,1, "airbrakes [0..1]")
VARDEF(uint,    ctrb,           0,1,   "controls bitfield [on/off]")
BITDEF(ctrb,  gear,      1, "Landing gear retracted/extracted")
BITDEF(ctrb,  brake,     2, "Parking brake on/off")
BITDEF(ctrb,  ers,       4, "Parachute launched/off")

//--------- AUTOPILOT COMMAND --------------
VARDEF(Vect,   cmd_theta,    -180,2,   "desired roll,pitch,yaw [deg]")
VARDEF(Vect,   cmd_NED,      -10000,2, "desired north,east,down [m]")
VARDEF(double, cmd_course,   -180,2,   "desired course [deg]")
VARDEF(double, cmd_rpm,      25500,1,  "desired RPM [rpm]")
VARDEF(double, cmd_airspeed, 0,1,      "desired airspeed (for regThr) [m/s]")
VARDEF(double, cmd_vspeed,   -12.7,1,  "desired vertical speed (for regPitchH) [m/s]")

//--------- WAYPOINTS --------------
VARDEF(uint,  wpcnt,     0,1,       "number of waypoints [0...]")
VARDEF(uint,  wpType,    0,1,       "current waypoint type [wp_type]")
VARDEF(uint,  wpidx,     0,1,       "current waypoint [0...]")
VARDEF(uint,  rwcnt,     0,1,       "number of runways[0...]")

//--------- calculated by Mandala::calc() --------------
VARDEF(Vect,  NED,     -10000,2, "north,east,down position [m]")
VARDEF(double,homeHDG, -180,2,   "heading to home position [deg]")
VARDEF(double,dHome,   0,2,      "distance to GCU [m]")
VARDEF(double,dWPT,    0,2,      "distance to waypoint [m]")
VARDEF(Vect,  dNED,    -10000,2, "delta north,east,down [m]")
VARDEF(Vect,  vXYZ,    -50,1,    "velocity bodyframe x,y,z [m/s]")
VARDEF(Vect,  dXYZ,    -10000,2, "delta bodyframe x,y,z [m]")
VARDEF(Vect,  aXYZ,    -100,1,   "accelerations by trajectory ax,ay,az [m/c2]")
VARDEF(double,crsRate, -50,1,    "trajectory course change rate [deg/s]")
VARDEF(double,wpHDG,   -180,2,   "current waypoint heading [deg]")
VARDEF(double,rwHDG,   -180,2,   "current runway heading [deg]")
VARDEF(double,rwDelta, -127,1,   "runway horizontal displacement [m]")

//--------- OTHER SENSORS--------------
VARDEF(double, rpm,     0,2,     "engine RPM [1/min]")
VARDEF(double, agl,       25.5,1,      "Above Ground Level altitude [m]")
VARDEF(double, fuel,      1.0,1,       "Fuel [0..1]")
VARDEFA(double,tsens,10,  120.0,1,     "temperature sensors [C]")

//=============================================================================
//--------- CONFIG --------------
// IDX START FROM idxCFG (200)
//-------------------------------
#ifndef CFGDEF
#define CFGDEF(atype,aname,aspan,abytes,around,adescr)
#endif
#ifndef CFGDEFA
#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) CFGDEF(atype,aname,aspan,abytes,around,adescr)
#endif
// PID variables (the first 3 vars are hardcoded in GCU ConfigModel)
CFGDEFA(Vect,       pidK,regCnt,   655.35,2,0.01, "PID coeffitients Kp,Ki,Kd")
CFGDEFA(Vect,       pidL,regCnt,   100,1,1,       "PID limits Lp,Li,Ld [%]")
CFGDEFA(double,     pidLo,regCnt,  100,1,1,       "PID out limits Lo [%]")

// other variables (added to ConfigModel) automatically.
// description format:
// <Group>: text (Vect axis names if any)
// Vect axis names used to create text names in ConfigModel
// <Group> may be ommited if repeated
// Vect arrays not allowed (by ConfigModel)
CFGDEF(double,  mix_elv_Kp,   2.55,1,0.01,  "Mixer: Roll angle to Elevator mix")
CFGDEF(double,  mix_elv_Lo,   100,1,1,      "Roll angle to Elevator limit")
CFGDEF(double,  mix_rud_Kp,   -12.7,1,0.1,  "Roll angle to Rudder mix")
CFGDEF(double,  mix_rud_Lo,   100,1,1,      "Roll angle to Rudder limit (+/-) [%]")
CFGDEF(double,  mix_thr_Kp,   25.5,1,0.1,   "Pitch angle to Throttle mix")
CFGDEF(double,  mix_thr_Lo,   100,1,1,      "Pitch angle to Throttle limit (+/-) [%]")

CFGDEF(Vect,    imu_align,    -180,2,0.1,   "AHRS: body frame align (roll,pitch,yaw) [-180..0..+180]")
CFGDEF(Vect,    theta_bias,   -180,2,0.1,   "AHRS: theta bias (roll,pitch,yaw) [-180..0..+180]")

CFGDEF(double,  wptSnap,      0,1,0,        "Waypoints: waypoint snap distance [m]")

CFGDEF(double,  distHome,   255000,1,1000,  "Safety: turn home distance [m]")
CFGDEF(double,  distKill,   255000,1,1000,  "Safety: self-kill distance [m]")

CFGDEF(double,  turnRate,   25.5,1,0.1,     "Sim: max turn rate [deg/s]")

CFGDEF(double,  hyst_dist,  25.5,1,0.1,     "Hysterezis: distance [m]")
CFGDEF(double,  hyst_spd,   25.5,1,0.1,     "speed hold [m/s]")
CFGDEF(double,  hyst_yaw,   25.5,1,0.1,     "heading hold [deg]")

CFGDEF(double,  eng_rpm,     25500,1,100,   "Engine: cruise rpm [1/min]")
CFGDEF(double,  eng_rpm_idle,25500,1,100,   "idle rpm [1/min]")
CFGDEF(double,  eng_rpm_max, 25500,1,100,   "max rpm [1/min]")


CFGDEF(double,  flight_speed,   0,1,0,      "Flight: cruise speed [m/s]")
CFGDEF(double,  flight_speedFlaps,0,1,0,    "airspeed limit with flaps down [m/s]")
CFGDEF(double,  flight_safeAlt, 2550,1,10,  "safe altitude, HOME, TA mode [m]")
CFGDEF(double,  flight_throttle, 0,1,0.01,  "cruise throttle setting [0..1]")

CFGDEF(double,  takeoff_Kp,      25.5,1,0.1,"Takeoff: alt error coeffitient [deg]")
CFGDEF(double,  takeoff_Lp,      90,1,1,    "pitch limit [deg]")
CFGDEF(double,  takeoff_throttle,1,1,0.1,   "throttle setting [0..1]")

CFGDEF(double,  pland_pitch,   90,1,1,    "Parachute Landing: stall pitch [deg]")
CFGDEF(double,  pland_speed,   255,1,1,   "airspeed to open parachute [m/s]")

CFGDEF(double,  rw_dist,     2550,1,10,   "Runway Landing: approach distance [m]")
CFGDEF(double,  rw_turnDist, -1270,1,10,  "approach turn distance [m]")
CFGDEF(double,  rw_finAGL,   255,1,0,     "approach final altitude AGL [m]")
CFGDEF(double,  rw_finPitch, -12.7,1,0.1, "final pitch bias [deg]")
CFGDEF(double,  rw_finSpeed, 0,1,0,       "final speed [m/s]")
CFGDEF(double,  rw_tdPitch,  0,1,1,       "touchdown pitch bias [deg]")
CFGDEF(double,  rw_tdAGL,    12.7,1,0.1,    "AGL altitude before touchdown [m]")
CFGDEF(double,  rw_Kp,       2.55,1,0.01, "error multiplier [m]")
CFGDEF(double,  rw_Lp,       255,1,1,     "heading limit [deg]")

CFGDEF(double,  stby_R,  2550,1,10,     "Standby mode: radius [m]")
CFGDEF(double,  stby_Kp, 2.55,1,0.01,   "error multiplier [m]")
CFGDEF(double,  stby_Lp, 255,1,1,       "heading limit [deg]")

CFGDEF(double,  flaps_speed,     2.55,1,0.01, "Flaps: flaps speed [0..1]")
CFGDEF(double,  flaps_level,     2.55,1,0.01, "flaps level [0..1]")
CFGDEF(double,  flaps_levelTO,   2.55,1,0.01, "flaps level for TAKEOFF mode [0..1]")
//=============================================================================
#undef MODEDEF
#undef REGDEF
#undef CMDDEF
#undef SIGDEF
#undef VARDEF
#undef VARDEFA
#undef BITDEF
#undef CFGDEF
#undef CFGDEFA
