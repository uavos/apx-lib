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
#ifndef __MANDALA_TEMPLATE_H
#define __MANDALA_TEMPLATE_H
//=============================================================================
// Mandala constants
//=============================================================================
#define idxPAD 32 //start index for regular vars
//-----------------------------------------------------------------------------
// Var Type enum
enum { vt_void = 0,
       vt_idx,
       vt_byte,
       vt_uint,
       vt_flag,
       vt_enum,
       vt_float,
       vt_vect,
       vt_point };
//=============================================================================
// Physical constants
#define EARTH_RATE 0.00007292115       // rotation rate of earth (rad/sec)
#define EARTH_RADIUS 6378137           // earth semi-major axis radius (m)
#define EARTH_MRADIUS 6371000.0        // earth mean radius (m)
#define ECCENTRICITY 0.0818191908426   // major eccentricity of earth ellipsoid
#define FLATTENING 0.0033528106650     // flattening of the ellipsoid
#define GRAVITY_0 9.7803730            // zeroth coefficient for gravity model
#define GRAVITY_1 0.0052891            // first coefficient for the gravity model
#define GRAVITY_2 0.0000059            // second coefficient for the gravity model
#define GRAVITY_NOM 9.81               // nominal gravity
#define SCHULER2 1.533421593170545E-06 // Sculer Frequency (rad/sec) Squared
#define R2D 57.29577951308232          // radians to degrees conversion factor
#define D2R 0.01745329251994           // degrees to radians conversion factor
#define FT2M 0.3048                    // feet to meters conversion factor
#define KTS2MS 0.5144                  // Knots to meters/sec conversion factor
#define PI 3.14159265358979            // pi
#define C_WGS84_a 6378137.0            // earth equitorial radius
#define C_WGS84_b 6356752.3142         // earth polor radius
#define C_WGS84_f ((C_WGS84_a - C_WGS84_b) / C_WGS84_a)
#define C_WIE 7.2321151467e-05 // WGS-84 earth rotation rate (rad/s)
//=============================================================================
#define VA_NUM_ARGS(...) (sizeof(#__VA_ARGS__) == sizeof("") ? 0 : VA_NUM_ARGS_IMPL(__VA_ARGS__, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))
#define VA_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, N, ...) N
//=============================================================================
#endif
//=============================================================================
//=============================================================================
//=============================================================================
//simple indexes
#ifndef MIDX
#define MIDX(...)
#endif
//------------------------------
// special protocols/indexes
//service packet, must be the first, i.e. var_idx = 0
MIDX(service, "Service packet <node_sn>,<cmd>,<data..>", = 0)
//special protocols
MIDX(xpdr, "Transponder data <xpdr>")
MIDX(ident, "Identification <ident>")
MIDX(dlink, "Datalink <squawk>,<var_idx>,<data..>")

MIDX(ping, "Ping packet, no reply expected", = 8)
MIDX(sim, "Simulator data <var_idx>,<data..>")
MIDX(hid, "HID data <var_idx>,<data..>")
MIDX(jsexec, "Execute JS script <script text>")
//mandala
MIDX(downstream, "Downlink stream <stream>", = 16)
MIDX(uplink, "Uplink wrapped packet <var_idx>,<data..>")
MIDX(mission, "Mission data <packed mission>")
MIDX(set, "Set component <var_idx>,<msk>,<float/byte>")
MIDX(data, "Port serial data <port_id>,<data..>")
MIDX(ldata, "Port data local <port_id>,<data..>")
MIDX(can, "Auxilary CAN data <ID32 b31=IDE>,<data..>")
MIDX(vmexec, "Execute VM script <@function>")
//other & temporary
MIDX(prio, "Priority assigned var <nodeID>,<prio>,<var_idx>,<data..>")
MIDX(active, "Autopilot active flag")
MIDX(video, "Video stream <data>")

MIDX(formation, "Formation flight data <packed data>")
MIDX(vor, "VOR beacon <packed data>")
//------------------------------
// idx LISTS
// send to bus on change
#define MANDALA_LIST_CTR idx_mode, idx_status, idx_error, idx_cmode, idx_power, idx_sw, idx_cam_ch, idx_cam_mode, idx_cam_opt, idx_cam_ctrb, idx_cams
// send to GCU
#define MANDALA_LIST_DLINK idx_xpdr, idx_ident, idx_dlink, idx_downstream, idx_ping, idx_mission, idx_data
//------------------------------
#undef MIDX
#ifndef MVAR
#define MVAR(...)
#endif
#ifndef MBIT
#define MBIT(...)
#endif
//=============================================================================
//    Mandala variables definitions
// type:           variable type: [float,  vect,   ...]
// name:           variable name, used for text also. access: var.name
// description:    text description of the variable, used mostly in GCU
// if bitfield description contains slash '/' then changes reported in dmsg
//------------------------------------------------------------------------------
// f2 - 2 bytes float
// f4 - 4 bytes float
// s1 - -127..+127
// s10 - -1270..+1270
// s001 - -1.27..+1.27
// u1 - uint8_t
// u2 - uint16_t
// u4 - uint32_t
// u01 - 0..25.5
// u001 - 0..2.55
// u10 - 0..2550
// u100 - 0..25500
//--------- IMU --------------
MVAR(vect, theta, "attitude: roll,pitch,yaw [deg]", f2, f2)
MVAR(vect, acc, "acceleration: Ax,Ay,Az [m/s2]", f2, s1)
MVAR(vect, gyro, "angular rate: p,q,r [deg/s]", f2, s10)
MVAR(vect, mag, "magnetic field: Hx,Hy,Hz [a.u.]", f2, s001)

//--------- FLIGHT CONTROL SENSORS --------------
MVAR(float, altitude, "Altitude [m]", f4, f4)
MVAR(float, airspeed, "Airspeed [m/s]", f2, f2)
MVAR(float, vspeed, "Vertical speed [m/s]", f2, f2)
MVAR(float, course, "Moving direction [deg]", f2, f2)

//--------- OTHER AP SENSORS --------------
MVAR(uint, rpm, "engine RPM [1/min]", u2, u2)
MVAR(float, agl, "Above Ground Level altitude [m]", f2, f2)
MVAR(float, slip, "slip [deg]", f2, s01)
MVAR(float, attack, "angle of attack [deg]", f2, s01)
MVAR(float, venergy, "compensated variometer [m/s]", f2, f2)
MVAR(float, ldratio, "glide ratio [Lift/Drag]", f2, f2)
MVAR(float, buoyancy, "Blimp ballonet pressure [kPa]", f2, f2)

#define idx_UPD_START idx_ctr_ailerons
//--------- FAST CONTROLS --------------
MVAR(float, ctr_ailerons, "ailerons [-1..0..+1]", f2, s001)
MVAR(float, ctr_elevator, "elevator [-1..0..+1]", f2, s001)
MVAR(float, ctr_throttle, "throttle [0..1]", f2, u001)
MVAR(float, ctr_rudder, "rudder [-1..0..+1]", f2, s001)
MVAR(float, ctr_collective, "collective pitch [-1..0..+1]", f2, s001)

//--------- SLOW CONTROLS --------------
MVAR(float, ctr_steering, "steering [-1..0..+1]", f2, s001)
MVAR(float, ctr_airbrk, "airbrakes [0..1]", f2, u001)
MVAR(float, ctr_flaps, "flaps [0..1]", u001, u001)
MVAR(float, ctr_brake, "brake [0..1]", f2, u001)
MVAR(float, ctr_mixture, "mixture [0..1]", f2, u001)
MVAR(float, ctr_engine, "engine tuning [0..1]", f2, u001)
MVAR(float, ctr_sweep, "sweep [-1..0..1]", f2, s001)
MVAR(float, ctr_buoyancy, "buoyancy [-1..0..1]", f2, s001)

MVAR(flag, ctrb, "controls bitfield [on/off]", u1, u1)
MBIT(ctrb, ers, "ERS on/off", 1)
MBIT(ctrb, rel, "Parachute released/locked", 2)
MBIT(ctrb, drp, "Drop-off open/locked", 4)
MBIT(ctrb, pump, "Fuel pump on/off", 8)
MBIT(ctrb, starter, "Starter request", 16)
MBIT(ctrb, horn, "Horn signal", 32)
MBIT(ctrb, rev, "Reverse activated/off", 64)
MBIT(ctrb, gear, "Landing gear retracted/extracted", 128)

MVAR(float, ctr_brakeL, "left brake [0..1]", f2, u001)
MVAR(float, ctr_brakeR, "right brake [0..1]", f2, u001)
#define idx_UPD_END idx_ctr_brakeR

//--------- AUTOPILOT COMMAND --------------
MVAR(vect, cmd_theta, "commanded attitude: roll,pitch,yaw [deg]", f2, f2)
MVAR(point, cmd_NE, "commanded position: north,east [m]", f2, f2)
MVAR(float, cmd_course, "commanded course [deg]", f2, f2)
MVAR(uint, cmd_rpm, "commanded RPM [rpm]", u2, u2)
MVAR(float, cmd_altitude, "commanded altitude [m]", f2, f2)
MVAR(float, cmd_airspeed, "commanded airspeed [m/s]", f2, f2)
MVAR(float, cmd_vspeed, "commanded vertical speed [m/s]", f2, s01)
MVAR(float, cmd_slip, "commanded slip [deg]", f2, s1)

//--------- GPS receiver --------------
MVAR(vect, gps_pos, "GPS position: lat,lon,hmsl [deg,deg,m]", f4, f4)
MVAR(vect, gps_vel, "GPS velocity: Vnorth,Veast,Vdown [m/s]", f2, f2)
MVAR(uint, gps_time, "GPS UTC Time from 1970 1st Jan [sec]", u4, u4)

//--------- OTHER SENSORS (information) --------------
MVAR(float, fuel, "fuel capacity [l]", f4, f2)
MVAR(float, frate, "fuel flow rate [l/h]", f4, f2)
MVAR(float, RSS, "modem signal strength [0..1]", u001, u001)

//--------- BATTERY --------------
MVAR(float, Ve, "system battery voltage [v]", f2, f2)
MVAR(float, Vs, "servo battery voltage [v]", f2, f2)
MVAR(float, Vp, "payload battery voltage [v]", f2, f2)
MVAR(float, Vm, "engine battery voltage [v]", f2, f2)
MVAR(float, Ie, "system current [A]", u001, u001)
MVAR(float, Is, "servo current [A]", u01, u01)
MVAR(float, Ip, "payload current [A]", u01, u01)
MVAR(float, Im, "engine current [A]", u1, u1)

//--------- TEMPERATURES --------------
MVAR(float, AT, "ambient temperature [C]", s1, s1)
MVAR(float, RT, "room temperature [C]", s1, s1)
MVAR(float, MT, "modem temperature [C]", s1, s1)
MVAR(float, ET, "engine temperature [C]", u1, u1)
MVAR(float, EGT, "exhaust gas temperature [C]", u10, u10)
MVAR(float, OT, "oil temperature [C]", u1, u1)
MVAR(float, OP, "oil pressure [atm]", u01, u01)

//--------- Instrument Landing System --------------
MVAR(flag, ilsb, "ILS bitfield [on/off]", u1, u1)
MBIT(ilsb, armed, "ILS armed/off", 1)
MBIT(ilsb, approach, "ILS approach available/lost", 2)
MBIT(ilsb, offset, "ILS offset available/lost", 4)
MBIT(ilsb, platform, "ILS platform available/lost", 8)
MVAR(point, ils_RSS, "ILS signal strength: IR,RF [0..1]", u001, u001)
MVAR(float, ils_HDG, "ILS heading to VOR1 [deg]", f2, f2)
MVAR(float, ils_DME, "ILS distance to VOR1 [m]", f2, f2)
MVAR(point, ils_offset, "ILS offset: heading,altitude [deg]", f2, f2)
MVAR(vect, platform_pos, "Platform position: lat,lon,hmsl [deg,deg,m]", f4, f4)
MVAR(vect, platform_vel, "Platform velocity: Vnorth,Veast,Vdown [m/s]", f2, f2)
MVAR(float, platform_hdg, "Platform heading [deg]", f2, f2)

//--------- Proximity & movement sensors --------------
MVAR(float, range, "distance to target [m]", f2, f2)
MVAR(vect, radar_vXYZ, "radar velocity: Vx,Vy,Vz [m/s]", f2, f2)
MVAR(vect, radar_dXYZ, "radar delta: dx,dy,dz [m]", f2, f2)

//--------- STATUS FLAGS --------------
MVAR(byte, stage, "maneuver stage", u1, u1)
MVAR(enum, mode, "flight mode", u1, u1)
MBIT(mode, EMG, "Realtime control", 0)
MBIT(mode, RPV, "Angles control", 1)
MBIT(mode, UAV, "Heading control", 2)
MBIT(mode, WPT, "Waypoints navigation", 3)
MBIT(mode, HOME, "Go back home", 4)
MBIT(mode, STBY, "Loiter around DNED", 5)
MBIT(mode, TAXI, "Taxi", 6)
MBIT(mode, TAKEOFF, "Takeoff", 7)
MBIT(mode, LANDING, "Landing", 8)

MVAR(flag, status, "AP status", u1, u1)
MBIT(status, rc, "RC on/off", 1)
MBIT(status, gps, "GPS available/lost", 2)
MBIT(status, home, "GPS initialized", 4)
MBIT(status, agl, "AGL available/off", 8)
MBIT(status, modem, "Uplink available/lost", 16)
MBIT(status, landed, "Vehicle landed/flying", 32)
MBIT(status, touch, "Landing gear touchdown/floating", 64)

MVAR(flag, error, "AP generated errors", u1, u1)
MBIT(error, power, "Power supply error/ok", 1)
MBIT(error, cas, "CAS error/ok", 2)
MBIT(error, pstatic, "Static pressure error/ok", 4)
MBIT(error, gyro, "IMU gyros bias", 8)
MBIT(error, rpm, "RPM sensor error/ok", 16)

MVAR(flag, cmode, "AP operation options", u1, u1)
MBIT(cmode, dlhd, "High precision downstream on/off", 1)
MBIT(cmode, thrcut, "Throttle cut on/off", 2)
MBIT(cmode, throvr, "Throttle override on/off", 4)
MBIT(cmode, hover, "Stabilization mode hover/run", 8)
MBIT(cmode, hyaw, "Hover yaw compensated/fixed", 16)
MBIT(cmode, ahrs, "AHRS mode inertial/gps", 32)
MBIT(cmode, nomag, "Magnetometer blocked/on", 64)
//MBIT(cmode,  glide,   "Glide mode on/off",                      128)

//--------- POWER CONTROL --------------
MVAR(flag, power, "power controls", u1, u1)
MBIT(power, ap, "Avionics on/off", 1)
MBIT(power, servo, "Servo on/off", 2)
MBIT(power, ignition, "Engine on/off", 4)
MBIT(power, payload, "Payload activated/off", 8)
MBIT(power, agl, "AGL sensor on/off", 16)
MBIT(power, xpdr, "XPDR on/off", 32)

MVAR(flag, sw, "switch controls", u1, u1)
MBIT(sw, starter, "Starter on/off", 1)
MBIT(sw, lights, "Lights on/off", 2)
MBIT(sw, taxi, "Taxi lights on/off", 4)
MBIT(sw, ice, "Anti-ice on/off", 8)
MBIT(sw, sw1, "switch 1 on/off", 16)
MBIT(sw, sw2, "switch 2 on/off", 32)
MBIT(sw, sw3, "switch 3 on/off", 64)
MBIT(sw, sw4, "switch 4 on/off", 128)

MVAR(flag, sb, "Event sensors [on/off]", u1, u1)
MBIT(sb, shutdown, "System shutdown/on", 1)
MBIT(sb, ers_err, "ERS error/ok", 2)
MBIT(sb, ers_disarm, "ERS disarmed/armed", 4)
MBIT(sb, eng_err, "Engine error/ok", 8)
MBIT(sb, bat_err, "Battery error/ok", 16)
MBIT(sb, gen_err, "Alternator error/ok", 32)

//--------- Flight Maneuvers --------------
MVAR(byte, wpidx, "current waypoint [0..255]", u1, u1)
MVAR(byte, rwidx, "current runway [0..255]", u1, u1)
MVAR(byte, twidx, "current taxiway [0..255]", u1, u1)
MVAR(byte, piidx, "current point of interest [0..255]", u1, u1)
MVAR(enum, midx, "mission index action", u1, u1)
MBIT(midx, inc, "Increment mission index", 0)
MBIT(midx, dec, "Decrement mission index", 1)
MVAR(float, tgHDG, "current tangent heading [deg]", f2, f2)
MVAR(float, turnR, "current circle radius [m]", f2, f2)
MVAR(float, delta, "general delta (depends on mode) [m]", f2, f2)
MVAR(byte, loops, "number of remaining turns or loops [0..255]", u1, u1)
MVAR(uint, ETA, "estimated time of arrival [s]", u4, u4)
MVAR(enum, mtype, "Mission maneuver type", u1, u1)
MBIT(mtype, hdg, "Heading navigation", 0)
MBIT(mtype, line, "Line navigation", 1)

//--------- dynamic tuning --------------
MVAR(float, windSpd, "wind speed [m/s]", f2, u01)
MVAR(float, windHdg, "wind direction to [deg]", f2, f2)
MVAR(float, cas2tas, "CAS to TAS multiplier [K]", f2, u001)
MVAR(float, rwAdj, "runway displacement adjust [m]", s1, s1)
MVAR(byte, gps_SV, "GPS Satellites visible [number]", u1, u1)
MVAR(byte, gps_SU, "GPS Satellites used [number]", u1, u1)
MVAR(byte, gps_jcw, "GPS CW Jamming level [0..255]", u1, u1)
MVAR(enum, gps_jstate, "GPS Jamming", u1, u1)
MBIT(gps_jstate, off, "disabled", 0)
MBIT(gps_jstate, ok, "ok", 1)
MBIT(gps_jstate, warn, "warning", 2)
MBIT(gps_jstate, critical, "critical warning", 3)

MVAR(vect, home_pos, "home position: lat,lon,hmsl [deg,deg,m]", f4, f4)
MVAR(float, altps_gnd, "barometric altitude on ground level [m]", f4, f4)
MVAR(byte, errcode, "error code", u1, u1)
MVAR(float, stab, "Stability [0..1]", f2, u001)

//--------- payloads --------------
MVAR(vect, cam_theta, "camera orientation: roll,pitch,yaw [deg]", f4, f2)
MVAR(point, turret_att, "turret orientation: pitch,heading [deg]", f4, f2)
MVAR(flag, turret, "turret controls bitfield [on/off]", u1, u1)
MBIT(turret, armed, "turret armed/disarmed", 1)
MBIT(turret, shoot, "turret shooting/standby", 2)
MBIT(turret, reload, "turret reloading/reloaded", 4)
MBIT(turret, sw1, "turret switch1 on/off", 8)
MBIT(turret, sw2, "turret switch2 on/off", 16)
MBIT(turret, sw3, "turret switch3 on/off", 32)
MBIT(turret, sw4, "turret switch4 on/off", 64)

//--------- user --------------
MVAR(flag, userb, "user bitfield [on/off]", u1, u1)
MBIT(userb, 1, "user bit 1", 1)
MBIT(userb, 2, "user bit 2", 2)
MBIT(userb, 3, "user bit 3", 4)
MBIT(userb, 4, "user bit 4", 8)
MBIT(userb, 5, "user bit 5", 16)
MBIT(userb, 6, "user bit 6", 32)
MBIT(userb, 7, "user bit 7", 64)
MBIT(userb, 8, "user bit 8", 128)
MVAR(float, user1, "User value 1", f4, f2)
MVAR(float, user2, "User value 2", f4, f2)
MVAR(float, user3, "User value 3", f4, f2)
MVAR(float, user4, "User value 4", f4, f2)
MVAR(float, user5, "User value 5", f4, f2)
MVAR(float, user6, "User value 6", f4, f2)

//--------- FORMATION --------------
MVAR(vect, ls_att, "left: roll,pitch,cp [deg]", f2, f2)
MVAR(float, ls_spd, "left: airspeed [m/s]", f2, f2)
MVAR(float, ls_ail, "left: ail to center [deg]", f2, f2)

MVAR(vect, rs_att, "right: roll,pitch,cp [deg]", f2, f2)
MVAR(float, rs_spd, "right: airspeed [m/s]", f2, f2)
MVAR(float, rs_ail, "right: ail to center [deg]", f2, f2)

MVAR(float, vshape, "wing V shape [deg]", f2, f2)
MVAR(float, cmd_vshape, "commanded wing V shape [deg]", f2, f2)

MVAR(float, lrs_croll, "console roll central [deg]", f2, f2)

//------------------------------------------------------------------------------
// Never sent by downstream
// Can be packed
// Some are calculated on downstream receive
//------------------------------------------------------------------------------
MVAR(byte, local, "local byte [0..255]", u1, u1)
//--------- calculated by Mandala::calc() --------------
MVAR(point, pos_NE, "local position: north,east [m]", f4, f4)
MVAR(point, vel_NE, "local velocity: north,east [m/s]", f4, f4)
MVAR(float, homeHDG, "heading to home position [deg]", f4, f4)
MVAR(float, dHome, "distance to GCU [m]", f4, f4)
MVAR(float, dWPT, "distance to waypoint [m]", f4, f4)
MVAR(point, vXY, "bodyframe velocity: Vx,Vy [m/s]", f4, f4)
MVAR(point, dXY, "bodyframe delta: dx,dy [m]", f4, f4)
MVAR(float, gSpeed, "ground speed [m]", f4, f4)
MVAR(float, wpHDG, "current waypoint heading [deg]", f4, f4)
MVAR(float, rwDelta, "runway alignment [m]", f4, f4)
MVAR(float, rwDV, "runway alignment velocity [m/s]", f4, f4)

//--------- internal --------------
MVAR(uint, dl_period, "downlink period [ms]", u2, u2)
MVAR(uint, dl_timestamp, "downlink timestamp [ms]", u4, u4)

//--------- local sensors --------------
MVAR(float, altps, "Barometric altitude [m]", f4, f4)
MVAR(float, vario, "Barometric variometer [m/s]", f4, f4)
MVAR(float, vcas, "Airspeed derivative [m/s^2]", f4, f4)
MVAR(float, denergy, "Venergy derivative [m/s^2]", f4, f4)

//--------- PILOT CONTROLS --------------
MVAR(byte, rc_override, "RC override [0..255]", u1, u1)
MVAR(float, rc_roll, "RC roll [-1..0..+1]", s001, s001)
MVAR(float, rc_pitch, "RC pitch [-1..0..+1]", s001, s001)
MVAR(float, rc_throttle, "RC throttle [0..1]", u001, u001)
MVAR(float, rc_yaw, "RC yaw [-1..0..+1]", s001, s001)

//--------- CAM CONTROL --------------
MVAR(byte, cam_ch, "video channel [0..255]", u1, u1)
MVAR(enum, cam_mode, "camera control mode", u1, u1)
MBIT(cam_mode, camoff, "camera off", 0)
MBIT(cam_mode, stab, "gyro stabilization", 1)
MBIT(cam_mode, position, "attitude position", 2)
MBIT(cam_mode, speed, "attitude speed control", 3)
MBIT(cam_mode, target, "target position tracking", 4)
MBIT(cam_mode, fixed, "fixed position", 5)
MBIT(cam_mode, track, "target CV tracking", 6)
MVAR(vect, camcmd_theta, "commanded camera orientation: roll,pitch,yaw [deg]", f4, f4)
MVAR(float, cam_zoom, "camera zoom level [0..1]", u001, u001)
MVAR(float, cam_focus, "camera focus [0..1]", u001, u001)
MVAR(vect, cambias_theta, "camera stability bias: roll,pitch,yaw [deg/s]", f4, f4)
MVAR(flag, cam_opt, "camera options [on/off]", u1, u1)
MBIT(cam_opt, PF, "picture flip on/off", 1)
MBIT(cam_opt, NIR, "NIR filter on/off", 2)
MBIT(cam_opt, DSP, "display information on/off", 4)
MBIT(cam_opt, FMI, "focus mode infinity/auto", 8)
MBIT(cam_opt, FM, "focus manual/auto", 16)
MBIT(cam_opt, laser, "rangefinder on/off", 32)
MVAR(byte, cam_src, "camera source", u1, u1)
MVAR(vect, cam_tpos, "camera tracking position: lat,lon,hmsl [deg,deg,m]", f4, f4)
MVAR(vect, camctr_theta, "camera servo: roll,pitch,yaw [-1..0..+1]", f4, f4)
MVAR(enum, cams, "camera shooting mode", u1, u1)
MBIT(cams, idle, "no shooting", 0)
MBIT(cams, shot, "single snapshot", 1)
MBIT(cams, dshot, "distance triggered snapshots", 2)
MVAR(flag, cam_ctrb, "camera controls bitfield [on/off]", u1, u1)
MBIT(cam_ctrb, shtr, "shutter trigger", 1)
MBIT(cam_ctrb, arm, "arm auto focus", 2)
MBIT(cam_ctrb, rec, "rec button", 4)
MBIT(cam_ctrb, zin, "zoom in button", 8)
MBIT(cam_ctrb, zout, "zoom out button", 16)
MBIT(cam_ctrb, aux, "auxilary button", 32)
MVAR(uint, cam_tperiod, "period/distance for shooting", u2, u2)
MVAR(uint, cam_timestamp, "timestamp [ms]", u4, u4) //must be last for gimbal

//--------- TURRET CONTROL --------------
MVAR(point, turretcmd_att, "commanded turret orientation: pitch,yaw [deg]", f4, f4)
MVAR(vect, turretctr_theta, "turret servo: roll,pitch,yaw [-1..0..+1]", f4, f4)
MVAR(enum, turret_mode, "turret mode", u1, u1)
MBIT(turret_mode, gyro, "gyro stabilized", 0)
MBIT(turret_mode, fixed, "relative encoded attitude", 1)
MVAR(point, turretenc_att, "turret encoders: pitch,yaw [deg]", f4, f4)

//--------- ATS CONTROL --------------
MVAR(point, atscmd_att, "commanded ATS orientation: pitch,yaw [deg]", f4, f4)
MVAR(point, atsctr_att, "ATS servo: pitch,yaw [-1..0..+1]", f4, f4)
MVAR(point, atsenc_att, "ATS encoders: pitch,yaw [deg]", f4, f4)
MVAR(enum, ats_mode, "ATS mode", u1, u1)
MBIT(ats_mode, track, "track UAV", 0)
MBIT(ats_mode, manual, "manual", 1)

//--------- GCU use only --------------
MVAR(float, gcu_RSS, "GCU modem signal strength [0..1]", u001, u001)
MVAR(float, gcu_Ve, "GCU battery voltage [v]", u01, u01)
MVAR(float, gcu_MT, "GCU modem temperature [C]", s1, s1)

#if !defined(MANDALA_TINY)
MVAR(float, VM1, "script value 1", f4, f4)
MVAR(float, VM2, "script value 2", f4, f4)
MVAR(float, VM3, "script value 3", f4, f4)
MVAR(float, VM4, "script value 4", f4, f4)
MVAR(float, VM5, "script value 5", f4, f4)
MVAR(float, VM6, "script value 6", f4, f4)
MVAR(float, VM7, "script value 7", f4, f4)
MVAR(float, VM8, "script value 8", f4, f4)
MVAR(float, VM9, "script value 9", f4, f4)
MVAR(float, VM10, "script value 10", f4, f4)
MVAR(float, VM11, "script value 11", f4, f4)
MVAR(float, VM12, "script value 12", f4, f4)
MVAR(float, VM13, "script value 13", f4, f4)
MVAR(float, VM14, "script value 14", f4, f4)
MVAR(float, VM15, "script value 15", f4, f4)
MVAR(float, VM16, "script value 16", f4, f4)
MVAR(float, VM17, "script value 17", f4, f4)
MVAR(float, VM18, "script value 18", f4, f4)
MVAR(float, VM19, "script value 19", f4, f4)
MVAR(float, VM20, "script value 20", f4, f4)
MVAR(float, VM21, "script value 21", f4, f4)
MVAR(float, VM22, "script value 22", f4, f4)
MVAR(float, VM23, "script value 23", f4, f4)
MVAR(float, VM24, "script value 24", f4, f4)
MVAR(float, VM25, "script value 25", f4, f4)
MVAR(float, VM26, "script value 26", f4, f4)
MVAR(float, VM27, "script value 27", f4, f4)
MVAR(float, VM28, "script value 28", f4, f4)
MVAR(float, VM29, "script value 29", f4, f4)
MVAR(float, VM30, "script value 30", f4, f4)
MVAR(float, VM31, "script value 31", f4, f4)
MVAR(float, VM32, "script value 32", f4, f4)
#endif
//=============================================================================
#undef MVAR
#undef MBIT
