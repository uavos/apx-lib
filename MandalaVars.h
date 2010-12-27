#ifndef MANDALA_CONSTANTS
#define MANDALA_CONSTANTS
//=============================================================================
// Mandala constants
//=============================================================================
#define maxVars 256
#define idxCFG  200     //configuration variables
#define idxSIG  190     //special variables - signatures
//-----------------------------------------------------------------------------
#define AHRS_FREQ       100      // AHRS Update [Hz]
#define GPS_FREQ        4       // GPS Update [Hz]
#define SERVO_FREQ      20      // Servo update rate [Hz]
#define TELEMETRY_FREQ  10      // Telemetry send rate [Hz]
#define SIM_FREQ        10      // Simulator servo send rate [Hz]
//-----------------------------------------------------------------------------
// Controls indexes
enum {ciRoll,ciPitch,ciThr,ciYaw,ciColl};
enum {ciFlaps=4,ciFlaperons,ciWheel};
enum {ciR,ciP,ciT,ciY,ciC};
enum {ciAilerons,ciElevator,ciThrottle,ciRudder,ciSw};
typedef enum {vt_void,vt_uint,vt_double,vt_Vect,vt_sig}_var_type;
//-----------------------------------------------------------------------------
#define   srvCnt    10    // servos cnt
#define   jswCnt    4     // joystick axes
#define   ppmCnt    5     // RC PPM channels cnt
//-----------------------------------------------------------------------------
// gps_status variable bitmask
#define gps_status_valid  1 // GPS coordinates are valid
#define gps_status_fix    2 // GPS fix received (4Hz) used by AHRS
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
REGDEF(Alt,      "Altitude to Collective Pitch")
REGDEF(Runway,   "Line Flight to Desired Course")
REGDEF(Circle,   "Loiter Flight to Desired Course")
//=============================================================================

//=============================================================================
// Autopilot commands.
// name, argsSize, alias, "description"
// names are accessed as enum "cmdName"
// argSize<0 means variable length (used only for link errors)
// alias used on GCU console only, set to enable command in GCU console
//=============================================================================
#ifndef CMDDEF
#define CMDDEF(aname,aargs,aalias,adescr)
#endif
CMDDEF(None,   0,,     "nothing")
CMDDEF(Mode,   1,,     "set flight mode")
CMDDEF(IMU,    0,imu,  "calibrate IMU alignment")
CMDDEF(NED,    0,ned,  "set home to current UAV position")
// flight control commands
CMDDEF(JSW,    jswCnt,, "joystick axis values 0..255 (cmd not acknowleged)")
CMDDEF(Turn,   4,turn,  "adjust heading [+/- deg]")
CMDDEF(Climb,  4,climb, "adjust altitude [+/- m]")

//special commands
CMDDEF(MandalaSet,     -1,,   "Mandala variables set [signature,vars] (uplink)")
CMDDEF(MandalaReq,     -1,,   "Mandala variables request [signature] (uplink/downlink)")
CMDDEF(TelemetrySet,   -1,telemetry,   "send telemetry signature")
CMDDEF(ACK,            -1,,   "send from uav to acknowlege last command")
CMDDEF(TelemetryData,  -1,,   "telemetry [data] (downlink)")
CMDDEF(Stdout,         -1,,   "text <stdout> from uav")
CMDDEF(WPT,            -1,,   "waypoints config, uplink")

CMDDEF(GOWPT,           4,gowpt,   "enter wpt mode, go to wpt#")

// IFC commands
CMDDEF(UartData,     -1,,   "send/receive data to IFC uart: [uart#(0..1)],[data...]")

//airframe
CMDDEF(Payload,    1,pld,    "payload on/off")
CMDDEF(Flaps,      1,flaps,  "flaps extract/retract")
CMDDEF(Gear,       1,gear,   "gear up/down")
CMDDEF(Brake,      1,brake,  "brake on/off")
CMDDEF(Int,        1,ic,     "extract/retract interceptors")
CMDDEF(Eng,        1,eng,    "start/stop engine")
CMDDEF(Parachute,  1,par,    "open/close parachute")

//=============================================================================
//-----------------------------------------------------------------------------
// special variables - signatures
#ifndef SIGDEF
#define SIGDEF(aname,...)
#endif
SIGDEF(imu, idx_acc, idx_gyro, idx_mag)
SIGDEF(gps, idx_gps_Lat, idx_gps_Lon, idx_gps_HMSL, idx_gps_course, idx_gps_velNED)
// dynamic signatures
SIGDEF(downlink)  //telemetry - can be rewritten
SIGDEF(uplink)    //telemetry - always zero (packet sent from GCU)
SIGDEF(config)    //configuration - list all vars>=idxCFG
//=============================================================================
//    Mandala variables definitions
// type:           variable type: [uint, double, Vect]
// name:           variable name, used for text also. access: var.name
// array:          for VARDEFA only (array size)
// span:           value span (for exchange compression), <0 = signed
// bytecount:      bytes number for data exchange compression
// description:    text description of the variable, used mostly in GCU
//------------------------------------------------------------------------------
#ifndef VARDEF
#define VARDEF(atype,aname,aspan,abytes,adescr)
#endif
#ifndef VARDEFA
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) VARDEF(atype,aname,aspan,abytes,adescr)
#endif
//--------- SPECIAL --------------
VARDEF(uint, mode, 0,1,       "mode of operation: EMG,RPV,UAV...")
VARDEF(uint, modeStage, 0,1,  "auto set to zero by mode change, wpt index")

//--------- IMU --------------
VARDEF(Vect, theta, -180,2,  "roll,pitch,yaw [deg]")
VARDEF(Vect, acc,   -100,2, "Ax,Ay,Az accelerations [m/s2]")
VARDEF(Vect, gyro,  -300,2, "p,q,r angular rates [deg/s]")
VARDEF(Vect, mag,   -2,1,   "Hx,Hy,Hz magnetic field vector [gauss]")

//--------- GPS --------------
VARDEF(uint,   gps_status,  0,1,    "gps status flags")
VARDEF(double, gps_Lat,     -180,4,     "latitude [deg]")
VARDEF(double, gps_Lon,     -180,4,     "longitude [deg]")
VARDEF(double, gps_HMSL,    -10000,2,   "altitde above sea [m]")
VARDEF(Vect, gps_NED,       -10000,2,   "north,east,down [m]")
VARDEF(Vect, gps_velNED,    -150,2,     "velocity north,east,down [m/s]")
VARDEF(Vect, gps_deltaNED,  -10000,2,   "distance to desired north,east,down [m]")
VARDEF(Vect, gps_velXYZ,    -150,2,     "velocity bodyframe x,y,z [m/s]")
VARDEF(Vect, gps_deltaXYZ,  -10000,2,   "velocity bodyframe x,y,z [m]")
VARDEF(Vect, gps_accXYZ,    -100,2,   "accelerations bodyframe ax,ay,az [m/c2]")
VARDEF(double, gps_distWPT, 0,2,    "distance to waypoint [m]")
VARDEF(double, gps_distHome,0,2,    "distance to GCU [m]")
VARDEF(double, gps_course,  -180,2,     "GPS course [deg]")
VARDEF(double, gps_crsRate, -50,1,      "GPS course change rate [deg/s]")
VARDEF(Vect, gps_accuracy, 2.55,1,     "GPS accuracy estimation Hrz[m],Ver[m],Spd[m/s]")
VARDEF(double, gps_home_Lat,     -180,4,     "home latitude [deg]")
VARDEF(double, gps_home_Lon,     -180,4,     "home longitude [deg]")
VARDEF(double, gps_home_HMSL,    -10000,2,   "home altitde above sea [m]")

//--------- BATTERY --------------
VARDEF(double, Ve,     655.35,2,     "autopilot battery voltage [v]")
VARDEF(double, Vs,     655.35,2,     "servo battery voltage [v]")
VARDEF(double, Vp,     0,2,     "payload battery voltage [v]")

//--------- TEMPERATURES --------------
VARDEF(double, AT,    -100,1,      "ambient temperature [deg C]")
VARDEF(double, ET,     0,1,       "engine temperature [deg C]")

//--------- CONTROLS --------------
VARDEFA(double, servo,srvCnt,  -1.0,1, "servo out [-1..0..+1]")
VARDEFA(double, jsw,jswCnt,    -1.0,1, "joystick command [-1..0..+1]")
VARDEFA(double, ppm,ppmCnt,    -1.0,1, "RC PPM command [-1..0..+1]")
VARDEF(uint,    jsw_valid,      0,1,  "joystick connected")


//--------- PRESSURE --------------
VARDEF(double, baro_airspeed,   100,1,    "barometric airspeed [m/s]")
VARDEF(double, baro_altitude,   10000,2,  "barometric altitude [m]")
VARDEF(double, baro_vspeed,     10,1,     "barometric vertical speed [m/s]")
VARDEF(uint,   baro_fix,     0,1,     "barometric sensor data received")

//--------- AUTOPILOT COMMAND --------------
VARDEF(Vect, desired_theta,   -180,2,   "desired roll,pitch,yaw [deg]")
VARDEF(Vect, desired_NED,     -10000,2, "desired north,east,down [m]")
VARDEF(double, desired_rpm,   0,2,  "desired RPM [rpm]")

//--------- GPIO --------------
VARDEF(uint, gpio,      0,1,       "GPIO bitfield [char]")
VARDEF(uint, gpio_cfg,  0,1,       "GPIO config (1=out) [char]")
VARDEF(uint, gpiov,     0,1,       "GPIO input [char]")

//--------- WAYPOINTS --------------
VARDEF(uint,  wptcnt,    0,1,       "waypoints number [0...]")

//--------- OTHER SENSORS--------------
VARDEF(double, rpm,     0,2,     "engine RPM [1/min]")
VARDEF(double, AGL,       25.5,1,      "Above Ground Level altitude [m]")
VARDEF(double, fuel,      1.0,1,       "Fuel [0..1]")
VARDEFA(double,tsens,10,  120.0,1,     "temperature sensors")

//--------- PAYLOAD --------------
VARDEFA(uint,   pld_status,     8,0,2,  "payload status [16 bits]")
VARDEFA(double, pld_values,     8,-1,4, "payload value [-1..+1]")
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

CFGDEF(uint,    servo_rev,    0,2,0,        "Servo: reverse [bitmap]")
CFGDEFA(double, servo_bias,srvCnt,  -1,1,0.01,  "[-1..0..+1]")

CFGDEF(Vect,    imu_align,    -180,2,0.1,   "AHRS: body frame align (roll,pitch,yaw) [-180..0..+180]")
CFGDEF(Vect,    theta_bias,   -180,2,0.1,   "AHRS: theta bias (roll,pitch,yaw) [-180..0..+180]")

CFGDEF(double,  wptSnap,      0,1,0,        "Waypoints: waypoint snap distance [m]")

CFGDEF(double,  distHome,   255000,1,1000,  "Safety: turn home distance [m]")
CFGDEF(double,  distKill,   255000,1,1000,  "Safety: self-kill distance [m]")

CFGDEF(double,  turnRate,   25.5,1,0.1,     "Sim: max turn rate [deg/s]")

CFGDEF(double,  hyst_dist,  25.5,1,0.1,     "Hysterezis: distance [m]")
CFGDEF(double,  hyst_spd,   25.5,1,0.1,     "speed hold [m/s]")
CFGDEF(double,  hyst_yaw,   25.5,1,0.1,     "heading hold [deg]")

CFGDEF(double,  flight_speed,   0,1,0,      "Flight: cruise speed [m/s]")
CFGDEF(double,  flight_rpm,     25500,1,100,"cruise rpm [1/min]")
CFGDEF(double,  flight_rpmIdle, 25500,1,100,"idle rpm [1/min]")
CFGDEF(double,  flight_safeAlt, 2550,1,10,  "safe altitude, HOME mode [m]")
CFGDEF(double,  flight_throttle,-1,1,0.01,  "cruise throttle setting [-1..0..+1]")
CFGDEF(double,  flight_stbyR,   2550,1,10,  "standby mode radius [m]")

CFGDEF(double,  takeoff_Kp,      25.5,1,0.1,"Takeoff: alt error coeffitient [deg]")
CFGDEF(double,  takeoff_Lp,      90,1,1,    "pitch limit [deg]")
CFGDEF(double,  takeoff_throttle,-1,1,0.1,  "throttle setting [-1..0..+1]")
CFGDEF(double,  takeoff_altitude,2550,1,10, "altitude to go WPT mode [m]")

CFGDEF(double,  landing_pitch,   90,1,1,    "Landing: stall pitch [deg]")
CFGDEF(double,  landing_speed,   255,1,1,   "airspeed to open parachute [m/s]")


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
#undef CFGDEF
#undef CFGDEFA
